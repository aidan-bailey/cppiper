#include "receiver.hh"
#include "sender.hh"
#include "cppiperconfig.hh"
#include <chrono>
#include <iostream>
#include <thread>
#include <tuple>
#include <string>


int main(int argc, char *argv[]) {
  std::cout << CPPIPER_VERSION_MAJOR << std::endl;
  cppiper::Sender server_sender("server");
  char* msg = new char[1]{'A'};
  for (int i = 0; i < 1000; i++)
    bool success = server_sender.send(msg, 1);
  delete [] msg;
  server_sender.terminate();
  return 0;
}

/*
int main(int argc, char *argv[]) {
  std::cout << argv[0] << " Version " << cppiper_VERSION_MAJOR << "."
            << cppiper_VERSION_MINOR << std::endl;
  std::cout << "Starting thread" << std::endl;
  cppiper::Sender server_sender("server");
  cppiper::Sender client_sender("client");
  sleep(5);
  std::cout << "Starting receiver" << std::endl;
  cppiper::Receiver server_receiver("client");
  cppiper::Receiver client_receiver("server");
  // char *in{new char[2]{'A', 'B'}};
  char in[3]{'A', 'B', 'C'};
  std::chrono::high_resolution_clock::time_point start =
      std::chrono::high_resolution_clock::now(); // get the time...
  for (int i = 0; i < 100000; i++) {
    server_sender.send(in, 3);
    auto res = client_receiver.receive();
    client_sender.send(std::get<0>(res), std::get<1>(res));
    res = server_receiver.receive();
  }
  std::chrono::high_resolution_clock::time_point end =
      std::chrono::high_resolution_clock::now(); //...get the time again
  std::chrono::duration<double, std::milli> timeRequired = (end - start);
  std::cout << "Finished in " << timeRequired.count() << "milliseconds"
            << std::endl;
  server_sender.terminate();
  client_sender.terminate();
  server_receiver.terminate();
  client_receiver.terminate();
  // delete [] in;
  //
  std::cout << "Finished..." << std::endl;
}
*/
