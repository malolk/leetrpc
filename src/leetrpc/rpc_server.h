#ifndef LEETRPC_RPC_SERVER_H__
#define LEETRPC_RPC_SERVER_H__

#include "leetrpc/dispatcher.h"
#include "network/tcp_server.h"
#include "network/socket_address.h"
#include "network/connection.h"
#include "libbase/buffer.h"
#include "libbase/loggerutil.h"
#include "libbase/noncopyable.h"

#include <string>
#include <functional>
#include <utility>
#include <memory>
#include <unordered_map>

namespace leetrpc {
class RpcServer: private libbase::Noncopyable {
 public:
  RpcServer(const std::string& ip = "127.0.0.1", 
            uint16_t port = 8080, int thread_num = 4) 
    : srv_(network::SocketAddress(ip, port), thread_num,
      std::bind(&RpcServer::ReadCb, this, 
                std::placeholders::_1, std::placeholders::_2)) {
  }

  void ReadCb(std::shared_ptr<network::Connection> c,
              libbase::ByteBuffer& data);
  void Register(Dispatcher* d);
  void Run() { srv_.Run(); }
  ~RpcServer();

 private:
  network::TcpServer srv_;
  std::unordered_map<int, Dispatcher*> context_;    
};
} // namespace leetrpc
#endif // LEETRPC_RPC_SERVER_H__

