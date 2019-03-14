#include "hydra/hydra_core.h"

#include <boost/algorithm/string.hpp>


namespace hydra {

// Real radio centre frequency, bandwidth; and the hypervisor's sampling rate, FFT size
HydraCore::HydraCore()
{
   // Initialise the resource manager
   p_resource_manager = std::make_unique<xvl_resource_manager>();
   p_hypervisor = std::make_unique<Hypervisor>();
}

void
HydraCore::set_rx_resources(uhd_hydra_sptr usrp,
                            double d_centre_freq,
                            double d_bandwidth,
                            unsigned int u_fft_size)
{
  // Initialise the RX resources
  p_resource_manager->set_rx_resources(d_centre_freq, d_bandwidth);
  p_hypervisor->set_rx_resources(usrp, d_centre_freq, d_bandwidth, u_fft_size);

  // Toggle flag
  b_receiver = true;
}

void
HydraCore::set_tx_resources(uhd_hydra_sptr usrp,
                            double d_centre_freq,
                            double d_bandwidth,
                            unsigned int u_fft_size)
{
   // Initialise the RX resources
   p_resource_manager->set_tx_resources(d_centre_freq, d_bandwidth);
   p_hypervisor->set_tx_resources(usrp, d_centre_freq, d_bandwidth, u_fft_size);

   // Toggle flag
   b_transmitter = true;
}

int
HydraCore::request_rx_resources(unsigned int u_id,
                                double d_centre_freq,
                                double d_bandwidth,
                                const std::string &server_addr,
                                const std::string &remote_addr)
{
  std::lock_guard<std::mutex> _p(g_mutex);


  // If not configured to receive
  if (not b_receiver)
  {
    // Return error -- zero is bad
    std::cout << "RX Resources not configured. Rejecting request." << std::endl;
    return 0;
  }

  auto vr = p_hypervisor->get_vradio(u_id);

  if(vr != nullptr and vr->get_rx_enabled())
  {
    // requesting tx resources for a VR already existing
    if (p_resource_manager->check_rx_free(d_centre_freq, d_bandwidth, u_id))
    {
      p_resource_manager->free_rx_resources(u_id);
      p_resource_manager->reserve_rx_resources(u_id, d_centre_freq, d_bandwidth);

      vr->set_rx_freq(d_centre_freq);
      vr->set_rx_bandwidth(d_bandwidth);

      return 1;
    }
  }

  // Try to reserve the resource chunks
  if(p_resource_manager->reserve_rx_resources(u_id, d_centre_freq, d_bandwidth))
  {
    // Return error -- zero is bad
    std::cout << "RX Resources already in use. Rejecting request." << std::endl;
    return 0;
  }

  // Create RX UDP port
  static size_t u_udp_port = 33000;
  if (vr == nullptr)
  {
      vr = std::make_shared<VirtualRadio>(u_id, p_hypervisor.get());
      p_hypervisor->attach_virtual_radio(vr);
  }

  vr->set_rx_chain(u_udp_port, d_centre_freq, d_bandwidth, server_addr, remote_addr);

   // If able to create all of it, return the port number
  std::cout << "RX Resources allocated successfully." << std::endl;
  return u_udp_port++;
}

int
HydraCore::request_tx_resources(unsigned int u_id,
                                double d_centre_freq,
                                double d_bandwidth,
                                const std::string &server_addr,
                                const std::string &remote_addr,
                                bool bpad)
{
  std::lock_guard<std::mutex> _p(g_mutex);

  // If not configured to transmit
  if (not b_transmitter)
  {
      // Return error -- zero is bad
      return 0;
  }

  // Tey to find the given VR
  auto vr = p_hypervisor->get_vradio(u_id);

  if(vr != nullptr and vr->get_tx_enabled())
  {
    // requesting tx resources for a VR already existing
    if (p_resource_manager->check_tx_free(d_centre_freq, d_bandwidth, u_id))
    {
      p_resource_manager->free_tx_resources(u_id);
      p_resource_manager->reserve_tx_resources(u_id, d_centre_freq, d_bandwidth);

      vr->set_tx_freq(d_centre_freq);
      vr->set_tx_bandwidth(d_bandwidth);

      return 1;
    }
  }

  /* Try to reserve the resource chunks */
  if(p_resource_manager->reserve_tx_resources(u_id, d_centre_freq, d_bandwidth))
  {
    return 0;
  }


  static size_t u_udp_port = 33500;
  if (vr == nullptr)
  {
     vr = std::make_shared<VirtualRadio>(u_id, p_hypervisor.get());
     vr->set_tx_chain(u_udp_port, d_centre_freq, d_bandwidth, server_addr, remote_addr, bpad);
     p_hypervisor->attach_virtual_radio(vr);
  }
  else
  {
    vr->set_tx_chain(u_udp_port, d_centre_freq, d_bandwidth, server_addr, remote_addr, bpad);
  }

  // If able to create all of it, return the port number
  return u_udp_port++;
}

// Query the virtual radios (and add their UDP port)
boost::property_tree::ptree
HydraCore::query_resources()
{  // Get the query from the RM
  boost::property_tree::ptree query_message = \
    p_resource_manager->query_resources();

  if (b_receiver)
  {
    // Get the RX child
    auto rx_chunks = query_message.get_child("receiver");
    // TODO There must be an easier way to do this
    query_message.erase("receiver");

    // Iterate over the child
    for (auto p_chunk = rx_chunks.begin();
         p_chunk != rx_chunks.end(); p_chunk++)
    {
      // If the current entry is a valid type
      if (p_chunk->second.get<int>("id"))
      {
         VirtualRadioPtr vr = p_hypervisor->get_vradio(p_chunk->second.get<int>("id"));

        // If receiving
         if ((vr != nullptr) and vr->get_rx_enabled())
        {
          // Add this new entry
           p_chunk->second.put("rx_udp_port",vr->get_rx_udp_port());
        }
      }
    }
    // Update the original tree
    query_message.add_child("receiver", rx_chunks);
  }

  if (b_transmitter)
  {
    // Get the TX child
    auto tx_chunks = query_message.get_child("transmitter");
    // TODO There must be an easier way to do this
    query_message.erase("transmitter");

    // Iterate over the child
    for (auto p_chunk = tx_chunks.begin();
         p_chunk != tx_chunks.end(); p_chunk++)
    {
      // If the current entry is a valid type
      if (p_chunk->second.get<int>("id"))
      {
        auto vr = p_hypervisor->get_vradio(p_chunk->second.get<int>("id"));

        // If receiving
        if ((vr != nullptr) and vr->get_tx_enabled())
        {
          // Add this new entry
           p_chunk->second.put("tx_udp_port",vr->get_tx_udp_port());
        }
      }
    }
    // Update the original tree
    query_message.add_child("transmitter", tx_chunks);
  }

  // Return updated
  return query_message;
}

// Deletes a given virtual radio
int
HydraCore::free_resources(size_t radio_id)
{
  std::cout << "CORE: freeing resources for radio: " << radio_id << std::endl;
  p_resource_manager->free_rx_resources(radio_id);
  p_resource_manager->free_tx_resources(radio_id);
  p_hypervisor->detach_virtual_radio(radio_id);

  std::cout << "CORE: DONE freeing resources for radio: " << radio_id << std::endl;
  return 1;
}

} // namespace hydra
