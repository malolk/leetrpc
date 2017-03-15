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

int main() {
  RpcClient client("127.0.0.1", 8080);
  AddClientStub stub(client);
  std::cout << stub.Add(1, 2) << std::endl;
  return 0;  
}


