#ifndef HYDRA_CLIENT_INCLUDE_H
#define HYDRA_CLIENT_INCLUDE_H

#include <string>
#include <iostream>
#include <sstream>

#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <zmq.hpp>

#include "hydra/util/udp.h"

namespace hydra
{


struct rx_configuration
{
   rx_configuration(double cf, double bw): center_freq(cf), bandwidth(bw) {};

   double center_freq;
   double bandwidth;
   int    server_port;
   std::string server_ip;
};



class hydra_client
{
public:
   /* CTOR
    */
   hydra_client(std::string client_ip = "localhost",
                unsigned int u_port = 5000,
                unsigned int u_client_id = 10,
                std::string s_group_name = "default",
                bool b_debug = false);

   /* DTOR
    */
   ~hydra_client();

   /* Request RX resources */
   int request_rx_resources(rx_configuration &rx_conf);

   /* Request TX resources */
   int request_tx_resources(rx_configuration &tx_conf);

   /* Check whether the hypervisor is alive */
   std::string check_connection(size_t max_tries = 10);

   /* Query the available resources */
   std::string query_resources();

   /* Free resources */
   std::string free_resources();


   void override_server_host(std::string s){ s_server_host = s; };

private:
   /* Base message methods */
   std::string factory(const std::string &s_message);
   int discover_server(std::string client, std::string &server_ip);


   std::string s_group;
   std::string s_client_host;
   std::string s_server_host;
   std::string s_server_port;

   /* Client ID -- TODO need a better way to define it */
   int u_id;

   /* Debug flag */
   bool b_debug_flag;
};


}; /* namespace hydra */

#endif
