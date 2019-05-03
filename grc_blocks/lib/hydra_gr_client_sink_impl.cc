#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "hydra_gr_client_sink_impl.h"

#include <gnuradio/io_signature.h>
#include <gnuradio/zeromq/push_sink.h>

namespace gr {
  namespace hydra {

hydra_gr_client_sink::sptr
hydra_gr_client_sink::make(unsigned u_id,
                           const std::string &host,
                           unsigned int port,
                           const std::string &s_group)
{
  return gnuradio::get_initial_sptr(
      new hydra_gr_client_sink_impl(u_id, host, port, s_group)
  );
}

/*
 * The private constructor
 */
hydra_gr_client_sink_impl::hydra_gr_client_sink_impl(
                   unsigned int u_id,
                   const std::string &s_host,
                   unsigned int u_port,
                   const std::string &s_group)
  :gr::hier_block2("gr_client_sink",
      gr::io_signature::make(1, 1, sizeof(gr_complex)),
      gr::io_signature::make(0, 0, 0))
{
  g_host = s_host;
  client = std::make_unique<hydra_client>(g_host, u_port, u_id, s_group, true);

  client->check_connection();
}

hydra_gr_client_sink_impl::~hydra_gr_client_sink_impl()
{
  client->free_resources();
}

void
hydra_gr_client_sink_impl::start_client(double d_center_frequency,
                                        double d_samp_rate,
                                        size_t u_payload)
{
  struct rx_configuration rx_conf{d_center_frequency, d_samp_rate};
  int err = client->request_tx_resources(rx_conf);

  if (!err)
  {
    // std::cout << boost::format("host: %s - port: %d") % g_host % rx_conf.server_port << std::endl;
#if 0
    connect(self(), 0, d_tcp_sink, 0);
#endif

#if 1
    std::string addr = "tcp://" + g_host + ":" + std::to_string(rx_conf.server_port);
    std::cout << "<hydra/sink> Server Address: " << addr << std::endl;
    gr::zeromq::push_sink::sptr d_sink = gr::zeromq::push_sink::make(sizeof(gr_complex),
                                                                     1,
                                                                     const_cast<char *>(addr.c_str()));

    connect(self(), 0, d_sink, 0);
#endif
    std::cout << "<hydra_sink> Client Sink initialized successfully." << std::endl;
  }
  else
  {
    std::cerr << "<hydra/sink> Not able to reserve resources." << std::endl;
  }
}


bool
hydra_gr_client_sink_impl::stop()
{
  client->free_resources();
}

int
hydra_gr_client_sink_impl::request_tx_resources(double d_center_frequency,
                                                double d_samp_rate,
                                                size_t u_payload)
{
   rx_configuration rx_conf{d_center_frequency, d_samp_rate};
   client->request_tx_resources(rx_conf);
   // TODO
}

  } /* namespace hydra */
} /* namespace gr */
