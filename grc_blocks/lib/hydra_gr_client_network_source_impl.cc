#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "hydra_gr_client_network_source_impl.h"

#include <gnuradio/io_signature.h>
#include <gnuradio/zeromq/pull_source.h>

namespace gr {
  namespace hydra {

hydra_gr_client_network_source::sptr
hydra_gr_client_network_source::make(unsigned  u_id,
                             const std::string &host_addr,
                             unsigned int      port,
                             const std::string &server_addr)
{
  return gnuradio::get_initial_sptr(new hydra_gr_client_network_source_impl(u_id, host_addr, port, server_addr));
}

/* CTOR
 */
hydra_gr_client_network_source_impl::hydra_gr_client_network_source_impl(unsigned int u_id,
                                                         const std::string &host_addr,
                                                         unsigned int u_port,
                                                         const std::string &server_addr):
  gr::hier_block2("gr_client_source",
                  gr::io_signature::make(0, 0, 0),
                  gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
  client = std::make_unique<hydra_client>(host_addr, u_port, u_id, true);
  client->override_server_host(server_addr);
}

hydra_gr_client_network_source_impl::~hydra_gr_client_network_source_impl()
{
  client->free_resources();
}

bool
hydra_gr_client_network_source_impl::stop()
{
  client->free_resources();
}

void hydra_gr_client_network_source_impl::start_client(double d_center_frequency,
                                               double d_samp_rate,
                                               size_t u_payload)
{
  rx_configuration rx_config{d_center_frequency, d_samp_rate, false};
  int err = client->request_rx_resources(rx_config);

  if (!err)
  {
    std::string addr = "tcp://" + rx_config.server_ip + ":" + std::to_string(rx_config.server_port);
    std::cout << "addr: " << addr << std::endl;
    gr::zeromq::pull_source::sptr d_source = gr::zeromq::pull_source::make(sizeof(gr_complex),
                                             1,
                                             const_cast<char *>(addr.c_str()));
    connect(d_source, 0, self(), 0);
  }
  else
  {
    std::cerr << "Not able to reserve resources." << std::endl;
    exit(1);
  }
}

  } /* namespace hydra */
} /* namespace gr */
