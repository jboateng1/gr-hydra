#include <boost/program_options.hpp>

#include "hydra/hydra_main.h"
#include "hydra/hydra_uhd_interface.h"
#include "hydra/hydra_hypervisor.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <mutex>
#include <vector>
#include <thread>

#include <sys/ipc.h>
#include <sys/shm.h>

#define PORT 8080
#define MAXLINE 1024

// newly_added
#define IPC_CREAT 01000

using namespace boost::program_options;
using namespace hydra;
using namespace std;

// Instantiate the backend object
hydra::uhd_hydra_sptr backend;

// Instantiate the HyDRA object
hydra::HydraMain *hydra_instance;
hydra::device_uhd *uhd_backend;
hydra::Hypervisor *aravisor;
hydra::VirtualRadio *container;
// hydra::VirtualRadio *container;

typedef std::vector<int> container_count;
typedef std::complex<float> complex1;
typedef std::complex<float> iq_sample;
typedef std::vector<complex1> iq_window;
typedef std::shared_ptr<hydra::abstract_device> uhd_hydra_sptr;

typedef std::shared_ptr<VirtualRadio> vradio_ptr;
typedef std::vector<vradio_ptr> vradio_vec;
// typedef std::unique_ptr<std::thread> tx_thread;

// typedef std::complex<float> complex;

// Generate a new 4-digit number, which is nowhere in previous (a 0-terminated array of ints) and fill string with that number
int generate_key(int *previous, char *string)
{
  int retval;
  while (1)
  {
    retval = rand() % 9999;
    int i;
    // if we already generated this number, make a new random value
    for (i = 0; previous[i]; i++)
    {
      if (previous[i] != retval)
      {
        continue;
      }
    }
    // add the new value to the previously seen array
    previous[i] = retval;
    break;
  }
  // fill string with the number
  if (string)
    sprintf(string, "%04d", retval);
  return retval;
}

void signal_handler(int signum)
{
  std::cout << "Closing Application." << std::endl;

  backend->release();
  hydra_instance->stop();
}

void tx_runn(iq_window iqptr, int num_iq, int tx_bw)
{
  size_t tx_sleep_time = llrint(num_iq * 1e6 / tx_bw);
  // iq_window optr(get_tx_fft());
  bool tx_thr = false;
  // Even loop

  while (not tx_thr)
  {
    // CHANGE send() to UHD not abstract device!!
    uhd_backend->send(iqptr, num_iq);
    // optr buffer is filled with zeros.
    // CHANGE it to the IFFT output of IQ SAMPLES!!
    // std::fill(optr.begin(), optr.end(), std::complex<float>(0, 0));
  }
}

int main(int argc, const char *argv[])
{
  signal(SIGINT, signal_handler);
  signal(SIGHUP, signal_handler);
  // signal(SIGQUIT, signal_handler);
  // signal(SIGABRT, signal_handler);

  // Boost Program Options configuration
  try
  {
    options_description desc{"Options"};
    desc.add_options()("help,h", "Help Message")("tx_freq", value<double>()->default_value(2e9), "Transmitter Centre Frequency")("tx_rate", value<double>()->default_value(1e6), "Transmitter Sampling Rate")("tx_gain", value<double>()->default_value(0.9), "Transmitter Normalized Gain")("tx_fft", value<double>()->default_value(1024), "Transmitter FFT Size")("rx_freq", value<double>()->default_value(2e9), "Receiver Centre Frequency")("rx_rate", value<double>()->default_value(1e6), "Receiver Sampling Rate")("rx_gain", value<double>()->default_value(0.0), "Receiver Normalized Gain")("rx_fft", value<double>()->default_value(1024), "Receiver FFT Size")("host", value<std::string>()->default_value("127.0.0.1"), "Server Host")("port", value<unsigned int>()->default_value(5001), "Server Port")("group", value<std::string>()->default_value("default"), "Hypervisor Group")("backend", value<std::string>()->default_value("usrp"), "Backend (usrp, loop, plot, file)");

    // Instantiate variables map, store, and notify methods
    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    // Print help and exit
    if (vm.count("help"))
    {
      std::cout << desc << '\n';
      return 0;
    }

    // Print debug information
    std::string delimiter = std::string(20, '-');
    std::cout << delimiter << " Transmitter Parameters " << delimiter << std::endl;
    std::cout << "Centre Frequency " << vm["tx_freq"].as<double>();
    std::cout << "\t"
              << "Sampling Rate " << vm["tx_rate"].as<double>();
    std::cout << std::setprecision(2) << "\t"
              << "Normalized Gain " << vm["tx_gain"].as<double>();
    std::cout << "\t"
              << "FFT Size " << vm["tx_fft"].as<double>() << "\n"
              << std::endl;
    std::cout << delimiter << "  Receiver Parameters   " << delimiter << std::endl;
    std::cout << "Centre Frequency " << vm["rx_freq"].as<double>();
    std::cout << "\t"
              << "Sampling Rate " << vm["rx_rate"].as<double>();
    std::cout << std::setprecision(2) << "\t"
              << "Normalized Gain " << vm["rx_gain"].as<double>();
    std::cout << "\t"
              << "FFT Size " << vm["rx_fft"].as<double>() << "\n"
              << std::endl;
    std::cout << delimiter << "    Server Parameters   " << delimiter << std::endl;
    std::cout << "Host " << vm["host"].as<std::string>();
    std::cout << "\t"
              << "Port " << vm["port"].as<unsigned int>();
    std::cout << "\t"
              << "Group " << vm["group"].as<std::string>() << "\n"
              << std::endl;

    // Extract the backend type
    std::string backend_type = vm["backend"].as<std::string>();

    /* Configure the backend */
    if (backend_type == "usrp")
    {
      backend = std::make_shared<hydra::device_uhd>();
      std::cout << "USRP Backend" << std::endl;
    }
    else if (backend_type == "loop")
    {
      backend = std::make_shared<hydra::device_loopback>();
      std::cout << "Loopback Backend" << std::endl;
    }
    else if (backend_type == "plot")
    {
      backend = std::make_shared<hydra::device_image_gen>();
      std::cout << "Plot Image Backend" << std::endl;
    }
    else if (backend_type == "file")
    {
      backend = std::make_shared<hydra::device_file>();
      std::cout << "File Backend" << std::endl;
    }
    else
    {
      // Throw error if the backend is not valid
      throw invalid_option_value("backend = " + backend_type);
    }

    /* TRANSMITTER */
    double d_tx_centre_freq = vm["tx_freq"].as<double>();
    double d_tx_samp_rate = vm["tx_rate"].as<double>();
    double d_tx_norm_gain = vm["tx_gain"].as<double>();
    double u_tx_fft_size = vm["tx_fft"].as<double>();
    // unsigned int u_tx_fft_size = vm["tx_fft"].as<unsigned int>();

    /* RECEIVER */
    double d_rx_centre_freq = vm["rx_freq"].as<double>();
    double d_rx_samp_rate = vm["rx_rate"].as<double>();
    double d_rx_norm_gain = vm["rx_gain"].as<double>();
    double u_rx_fft_size = vm["rx_fft"].as<double>();
    // unsigned int u_rx_fft_size = vm["rx_fft"].as<unsigned int>();

    /* Control port */
    unsigned int u_port = vm["port"].as<unsigned int>();
    std::string s_host = vm["host"].as<std::string>();
    std::string s_group = vm["group"].as<std::string>();

    /* Instantiate XVL */
    hydra_instance = new hydra::HydraMain(
        s_host + ":" + std::to_string(u_port),
        s_group);

    uhd_backend = new hydra::device_uhd();
    aravisor = new hydra::Hypervisor();
    container = new hydra::VirtualRadio();

    std::cout << "Testing123..." << std::endl;
    // Initializes the USRP driver with set transmit parameters

    uhd_hydra_sptr backend;
    uhd_backend->set_tx_config(d_tx_centre_freq, d_tx_samp_rate, d_tx_norm_gain);
    uhd_backend->set_rx_config(d_rx_centre_freq, d_rx_samp_rate, d_rx_norm_gain);
    /*hydra_instance->set_tx_config(
        backend,
        d_tx_centre_freq,
        d_tx_samp_rate,
        d_tx_norm_gain,
        u_tx_fft_size);
    */

    // Initializes the USRP driver with set receive parameters

    /*hydra_instance->set_rx_config(
        backend,
        d_rx_centre_freq,
        d_rx_samp_rate,
        d_rx_norm_gain,
        u_rx_fft_size);
    */
    /////////////////////////******************************/////////////////////////////////////////

    // Server side implementation of UDP client-server model

    // Driver code
    int sockfd;
    char buffer[MAXLINE];
    // char shm_key1[16] = {0};
    // char shm_key2[16] = {0};
    char response_buffer[256];
    int previous_keys[16] = {0};
    struct sockaddr_in servaddr = {0}, cliaddr = {0};

    char c1, c2;
    int shmid1, shmid2;
    key_t shm_key1, shm_key2;

    // This was char initially not float!
    iq_sample *shm1, *s1, *shm2, *s2;

    srand(time(NULL));

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror("socket creation failed");
      exit(EXIT_FAILURE);
    }

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0)
    {
      perror("bind failed");
      exit(EXIT_FAILURE);
    }
    // while (1)
    //{

    socklen_t len, n;

    len = sizeof(cliaddr); // len is value/result
    printf("Waiting for connection...\n");

    n = recvfrom(sockfd, buffer, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&cliaddr,
                 &len);

    buffer[n] = '\0';
    printf("[Container] : %s\n", buffer);

    const size_t SHMSZ = strtol(strstr(buffer, "\n") + 1, NULL, 10);

    shm_key1 = generate_key(previous_keys, NULL);
    shm_key2 = generate_key(previous_keys, NULL);

    // Create shared memory segment for Transmit Chain

    /*
     * We'll name our shared memory segment
     * using the key generated by server for hypervisor-->container chain.
     */
    if ((shmid1 = shmget(shm_key1, SHMSZ, IPC_CREAT | 0666)) < 0)
    {
      perror("shmget");
      exit(1);
    }

    /*
     * Now we attach the segment to our data space.
     */
    if ((shm1 = (iq_sample *)shmat(shmid1, NULL, 0)) == (iq_sample *)-1)
    {
      perror("shmat");
      exit(1);
    }

    // Create shared memory segment for Receive Chain
    /*
     * We'll name our shared memory segment
     * using the key generated by server for container-->hypervisor chain.
     */
    if ((shmid2 = shmget(shm_key2, SHMSZ, IPC_CREAT | 0666)) < 0)
    {
      perror("shmget");
      exit(1);
    }
    /*
     * Now we attach the segment to our data space.
     */
    if ((shm2 = (iq_sample *)shmat(shmid2, NULL, 0)) == (iq_sample *)-1)
    {
      perror("shmat");
      exit(1);
    }

    // store the tx and rx share memory segment keys/names in a response buffer
    sprintf(response_buffer, "%04d %04d", shm_key1, shm_key2);

    printf("[ARAvisor]: Sending response: %s\n", response_buffer);
    sendto(sockfd, response_buffer, strlen(response_buffer),
           MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
           len);
    sleep(1);
    char msg[] = "[ARAvisor]: Shared memory segment keys sent to container";
    long count = strtol(buffer + strlen("Container "), NULL, 10);

    size_t num_iq = SHMSZ / sizeof(iq_sample);

    iq_window iqptr(524288);
    bool thr_tx_stop = false;

    printf("size of iq_window: %ld\n", sizeof(iq_window));
    printf("size of iq_sample: %ld\n", sizeof(iq_sample));
    printf("size of float: %ld\n", sizeof(float));
    std::cout << "sample: " << iqptr[1022] << "\n";
    std::cout << "iqptr_size: " << iqptr.size() << "\n";
    std::cout << "num_iq: " << num_iq << "\n";
    sleep(4);
    for (int i = 0; i < 20; i++)
    {
      iqptr[i] = shm1[i];
      iqptr[i].real(shm1[i].real());
      iqptr[i].imag(shm1[i].imag());

      printf("%f %fj\n", iqptr[i].real(), iqptr[i].imag());
      // printf("\n");
    }
    printf("*****************************************************\n");
    printf("%s %ld\n", msg, count);
    //}
    std::cout << "iqptr front: \n"
              << iqptr.front() << "Buffer Size: \n"
              << iqptr.size() << "\n";

    iq_map_vec aravisor_subcarrier_map; // how do you see/print this?

    // figure out how to iterate this and specify the different frequencies
    // container->set_container_tx_chain(2000.2e6, 0.2e6, u_tx_fft_size, d_tx_samp_rate, *shm1);

    std::mutex vradios_mtx;
    iq_window iq_toradio(u_tx_fft_size);

    for (int i = 0; i < 3; i++)
    {
      // ARAvisor;
      size_t ID = aravisor->create_container(2000.2e6, 0.2e6, d_tx_samp_rate, u_tx_fft_size);
      hydra_buffer<iq_sample> *iq_buf = new hydra_buffer<iq_sample>(num_iq);
      for (int j = 0; j < num_iq; j++) {
        (*iq_buf)[j] = shm1[j];
      }
      std::shared_ptr<hydra_buffer<iq_sample>> buf_ptr = std::make_unique<hydra_buffer<iq_sample>>(iq_buf);
      container->set_container_tx_chain(2000.2e6, 0.2e6, u_tx_fft_size, d_tx_samp_rate, buf_ptr);
      std::cout << "containerID: " << ID << "\n";

      iq_map_vec subcarriers_map(u_tx_fft_size, -1);
      std::lock_guard<std::mutex> _l(vradios_mtx);
      std::cout << "[ARAvisor]: TX setting map for Container " << std::to_string(ID) << "\n";
      aravisor_subcarrier_map = subcarriers_map;

      size_t con_fft = aravisor->set_container_tx_mapping(aravisor_subcarrier_map, 0.2e6, 2000.2e6, d_tx_samp_rate, d_tx_centre_freq, u_tx_fft_size, ID);
      iq_map_vec the_map = aravisor->map_return(ID);

      iq_toradio = aravisor->get_aravisor_tx_window(iq_toradio, u_tx_fft_size, ID, the_map, con_fft);

    }

    // num_iq = fft
    ///////////////////////////*******************************//////////////////////////////////////
    // std::unique_ptr<std::thread> tx_thread;
    // tx_thread = std::make_unique<std::thread>(tx_runn,(iqptr, num_iq, d_tx_samp_rate));
    tx_runn(iqptr, num_iq, d_tx_samp_rate);
    // backend->send(iqptr, num_iq);
    //  aravisor->send(iqptr, num_iq);

    /* Run server */
    //////WAITS FOR CLIENT CONNECTIONS WITH RESOURCE SPECS///////
    hydra_instance->run();

    //////////////**********************************///////////////////////

    // Receive freq and bandwidth from Controller?
    // TODO change the fixed values..
    // size_t ID = aravisor->create_vradio(2.40e6,0.2e6);

    /*

    void
VirtualRadio::set_tx_mapping(const iq_map_vec &iq_map)
{
  g_tx_map = iq_map;
}

bool
VirtualRadio::map_tx_samples(iq_sample *samples_buf)
{
  // If the transmitter chain was not defined
  if (not b_transmitter){return false;}

  // Try to get a window from the resampler. Necessary??Yes
  iq_window buf  = tx_resampler->buffer()->read_one();

  // Return false if the window is empty
  if (buf.empty()){return false;}

  // Copy samples in TIME domain to FFT buffer, execute FFT
  //GO to IQ Gen and take the array of complex numbers HERE!!!
  g_fft_complex->set_data(&buf.front(), g_tx_fft_size);

  //MEASURE LATENCY of FFT here!!

  //Begin Time
  g_fft_complex->execute();
  //End Time


  //Frequency domain IQ samples
  iq_sample *outbuf = g_fft_complex->get_outbuf();

  // map samples in FREQ domain to samples_buff
  // performs fft shift
  int idx = 0;
  for (auto it = g_tx_map.begin(); it != g_tx_map.end(); ++it, ++idx)
  {
    samples_buf[*it] = outbuf[idx];
  }

  return true;
}

    size_t
  Hypervisor::get_tx_window(iq_window &optr, size_t len)
  {
    // If there's nothing to transmit, return 0
    if (g_vradios.size() == 0)
    {
      return 0;
    }

    {
      g_ifft_complex->reset_inbuf();

      std::lock_guard<std::mutex> _l(vradios_mtx);

      std::fill(g_ifft_complex->get_inbuf(), // Need explanation??
                g_ifft_complex->get_inbuf() + len,
                std::complex<float>(0, 0)); // hypervisor payload complex array of all IQ SAMPLES

      // Freq Domain IQ sample mapping from various vRadios taking place!!

      for (auto it = g_vradios.begin(); it != g_vradios.end(); ++it)
      {
        if ((*it)->get_tx_enabled())
          (*it)->map_tx_samples(g_ifft_complex->get_inbuf());
      }
    }

    g_ifft_complex->execute();

    // After IFFT is executed, the Time-Domain multiplexed samples are sent to the output BUFFER of hypervisor READY to be sent to UHD

    optr.assign(g_ifft_complex->get_outbuf(),
                g_ifft_complex->get_outbuf() + len);

    return len;
  }

  // Create new resampler
   tx_resampler = std::make_unique<resampler<iq_sample, iq_window>>(
       tx_socket->buffer(),
       d_tx_bw,
       g_tx_fft_size);

   // create fft object
   g_fft_complex  = sfft_complex(new fft_complex(g_tx_fft_size));
    */
  }
  catch (const error &ex)
  {
    std::cerr << ex.what() << '\n';
  }

  /* Run server */
  return 0;
}
