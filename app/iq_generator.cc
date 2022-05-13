#include <iostream>
#include <complex>
#include <string>
#include <chrono>
#include <array>
#include <thread>
#include <math.h>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include "hydra/hydra_client.h"

typedef std::complex<float> iq_sample;
typedef std::vector<iq_sample> iq_window;
using boost::asio::ip::udp;

#define PORT 8080
#define MAXLINE 1024

/*
class UDPClient
{
private:
  boost::asio::io_service& io_service_;
  udp::socket socket_;
  udp::endpoint endpoint_;

public:
  // Constructor
  UDPClient(boost::asio::io_service& io_service,
            const std::string host,
            const std::string port
  ) : io_service_(io_service), socket_(io_service, udp::endpoint(udp::v4(), 0))
  {
    udp::resolver resolver(io_service_);
    udp::resolver::query query(udp::v4(), host, port);
    udp::resolver::iterator iter = resolver.resolve(query);
    endpoint_ = *iter;
  }
  // Destructor
  ~UDPClient()
  {
    socket_.close();
  }

  // Send data constantly
  template<long unsigned int SIZE> // Using a template as we don t know the
  void send(std::array<std::complex<double>, SIZE>& msg, double rate, double loop)
  {
    // Get the time between frame
    long int threshold = lrint(msg.size() / rate);

    size_t counter = 0;
    do
    {
      // Sleep and wait for the right time

      std::this_thread::sleep_for(std::chrono::microseconds(threshold));
      // Send the information
      std::cout << "Sending samples " << counter++
                << ", msg size: " << msg.size()
                << std::endl;
      socket_.send_to(boost::asio::buffer(msg, sizeof(msg[0]) * msg.size()), endpoint_);
    } while (loop);
  }
};
*/

int main(int argc, char *argv[])
{
  int sockfd;
  char buffer[MAXLINE];
  //  const char *shm_key = "Container 1 requesting shared memory segment...";
  char shm_request[256];
  struct sockaddr_in servaddr = {0};

  int shmid1, shmid2;
  key_t shm_key1, shm_key2;
  iq_sample *shm1, *s1, *shm2, *s2;

  // printf("Keys: %04d %04d\n", shm_key1, shm_key2);

  //*******For Receive chain(hypervisor-->container)*******
  /*
   * We need to get the segment named
   * shm_key1, created by the server.
   */
  // Locate the shared memory segment for RX

  ///////////////////////////////////////////////////////
  // Default variables
  std::string host = "localhost";
  std::string port = "5000";
  std::string rate = "1e6"; // In Msps - defaults to 1 sample per second
  std::string file = "";
  const unsigned int p_size = 100;

  // Shittiest way to parse CLI argument ever, but does the drill
  if (argc % 2 == 0)
  {
    std::cout << "Wrong number of arguments" << std::endl;
    return 1;
  }

  // Iterate over the arguments and change the default variables accordingly
  for (int i = 1; i < argc; i += 2)
  {
    std::cout << argv[i] << std::endl;
    if (strcmp(argv[i], "-h") == 0)
      host = argv[i + 1];
    else if (strcmp(argv[i], "-p") == 0)
      port = argv[i + 1];
    else if (strcmp(argv[i], "-r") == 0)
      rate = argv[i + 1];
    else if (strcmp(argv[i], "-f") == 0)
      file = std::string(argv[i + 1]);

    std::cout << "i: " << i << ": " << argv[i] << std::endl;
  }

  /*
    // Request resources
    std::string lalala;
    hydra::hydra_client s1 = hydra::hydra_client("127.0.0.1", 5000, "default", true);

    hydra::rx_configuration tx_conf{1.1e9 + 500e3, std::stof(rate)};
    lalala = s1.request_tx_resources(tx_conf);
    std::cout << lalala << std::endl;

    // Initialise the async IO service
    boost::asio::io_service io_service;
    UDPClient client(io_service, tx_conf.server_ip, std::to_string(tx_conf.server_port));
  */
  std::cout << "FFT Size: " << p_size << "\tSampling rate: " << rate << "\tThreshold: " << p_size * 1e6 / std::stod(rate) << std::endl;
  // Construct the payload array
  std::array<std::complex<double>, p_size> payload;
  // Fill the array with IQ samples

  if (file == "")
  {
    payload.fill(std::complex<double>(2.0, 1.0)); // Need explanation
    // Send the payload at the given rate
    // client.send(payload, std::stod(rate), true);
  }
  else
  {
    std::ifstream fin(file, std::ios::binary);
    if (!fin)
    {
      std::cout << " Error, Couldn't find the file"
                << "\n";
      return 0;
    }

    

    fin.seekg(0, std::ios::end);
    const size_t num_elements = fin.tellg() / sizeof(iq_sample);
    std::string sk = std::to_string(num_elements);
    printf("%s\n", sk.c_str());
    // std::cout << sk;

    const size_t SHMSZ = num_elements * sizeof(iq_sample);
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror("socket creation failed");
      exit(EXIT_FAILURE);
    }

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //servaddr.sin_addr.s_addr = INADDR_ANY;

    int n, len;

    snprintf(shm_request, 256, "Container %d requesting shared memory\n%lu", 1, SHMSZ);
    sendto(sockfd, (const char *)shm_request, strlen(shm_request),
           MSG_CONFIRM, (const struct sockaddr *)&servaddr,
           sizeof(servaddr));
    printf("[Container] : Shared memory segment request sent.\n");

    n = recvfrom(sockfd, buffer, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                 (socklen_t *)&len);
    close(sockfd);

    buffer[n] = '\0';
    printf("[ARAvisor] : %s\n", buffer);
    shm_key1 = strtol(buffer, NULL, 10);
    shm_key2 = strtol(strstr(buffer, " ") + 1, NULL, 10);

    if ((shmid1 = shmget(shm_key1, SHMSZ, 0666)) < 0)
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
    /*
     * Now read what the server put in the memory.
     */
    // printf("********Receive Buffer********\n");
    // for (s1 = shm1; *s1 != '\0'; s1++)
    // putchar(*s1);
    // putchar('\n');
    //*******For Transmit chain(container-->hypervisor)*******
    /*
     * We need to get the segment named
     * shm_key2, created by the server.
     */
    // Locate the shared memory segment for TX
    if ((shmid2 = shmget(shm_key2, SHMSZ, 0666)) < 0)
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

    fin.seekg(0, std::ios::beg);
    iq_window data(num_elements);
    //std::vector<complex> data(num_elements);
    //fin.read(reinterpret_cast<char *>(&data), num_elements * sizeof(iq_window));
    fin.read(reinterpret_cast<char *>(&data[0]), num_elements * sizeof(iq_sample));

    std::cout << "sample: " << data[1] << "\n";
    s1 = shm1;
    for(int i=0; i < data.size(); i++)
    {
      //std::cout << "sample: " << data[i] << "\n";
      shm1[i].real(data[i].real());
      shm1[i].imag(data[i].imag());
      //std::cout << "sample: " << shm1[i] << "\n";

  
      //shm1[i] = data[i];
      //printf("sample: %d\n",data[i]);
    }

  /*
    for (int i = 0; i < data.size(); i++)
    {
      // std::cout << '(' << data[i] << ", " << data[++i] << ')' << ' ';
      // float *shared_memory;
      //printf("first elememt: %d\n", data[i]);
      shm1[2*i] = data[i].real();
      shm1[2*i+1] = data[i].imag();
      // shared_memory should have num_elements*sizeof(float) bytes
    }*/

    std::cout << std::endl;

    size_t counter = 0;
    while ((counter + p_size) < num_elements)
    {
      std::copy(&data[counter], &data[counter + p_size], payload.begin());
      // client.send(payload, std::stod(rate), false);
      counter += p_size;
    }
  }
}
