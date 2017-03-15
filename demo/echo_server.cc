#include "leetrpc/rpc_server.h"
#include "libbase/buffer.h"
#include "add_server_stub_def.h"

#include <iostream>
#include <string>
#include <memory>

#include <unistd.h>  // for getopt
#include <stdlib.h>  // for atoi

using namespace network;
using namespace libbase;
using namespace leetrpc;

int main(int argc, char* argv[]) {
  std::string ip_str = "127.0.0.1";
  uint16_t port = 8080;
  int thread_num = 4;
  int opt;
  while ((opt = getopt(argc, argv, "s:p:t:")) != -1) {
    switch (opt) {
      case 's': {
        ip_str = std::string(optarg);
        break;
      }
      case 'p': {
        port = static_cast<uint16_t>(atoi(optarg));
        break;
      }
      case 't': {
        thread_num = atoi(optarg);
        break;
      }
      default:  {
        std::cerr << "usage: " 
                     "-s <ip> -p <port> -t <thread num>"
                  << std::endl;
        return 1; 
      }
    }    
  }
  RpcServer srv(ip_str, port, thread_num);
  srv.Register(new AddServerStubDef()); 
  std::cout << "===========echo server============" << std::endl;
  srv.Run();

  return 0;
}
