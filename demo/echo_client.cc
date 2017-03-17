#include "leetrpc/rpc_client.h"
#include "add_client_stub.h"
#include "libbase/buffer.h"

#include <memory>
#include <iostream>
#include <string>

#include <unistd.h>

using namespace network;
using namespace libbase;
using namespace leetrpc;

void Print(std::shared_ptr<double> ret) {
  std::cout << "use asyn function: ret = " << *ret << std::endl;
}

int main() {
  RpcClient client("127.0.0.1", 8080);
  AddClientStub stub(client);
  std::cout << stub.Add(1, 2, 2) << std::endl;
  std::cout << stub.Add(1, 2) << std::endl;
  stub.Add(2, 3, Print, 2);
  stub.Add(2, 3, Print);
  std::cout << "Call asyn Add" << std::endl;
  sleep(10);
  return 0;  
}


