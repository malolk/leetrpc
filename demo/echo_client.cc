#include "leetrpc/rpc_client.h"
#include "leetrpc/rpc_status.h"
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
  RpcStatus rs;
  rs.TimeoutNotify();
  int ret = stub.Add(1, 2, &rs, 2);
  if (rs.Ok()) {
    std::cout << ret << std::endl;
  } else {
    std::cout << rs.ToString() << std::endl;
  }
  std::cout << stub.Add(1, 2) << std::endl;
    stub.Add(2, 4, Print, 2);
    stub.Add(2, 3, Print);
    std::cout << "Call asyn Add" << std::endl;
    sleep(6);
  return 0;  
}


