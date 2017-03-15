#include "leetrpc/rpc_client.h"
#include "libbase/loggerutil.h"

#include <stdlib.h>

namespace leetrpc {
    
void RpcClient::GenericInitCb(std::shared_ptr<network::Connection> c, 
                              libbase::ByteBuffer& data) {
  libbase::ByteBuffer length;
  length.AppendString(std::to_string(data.ReadableBytes()) + "#");
  c->Send(length);
  c->Send(data);
  data.Reset();
}

void RpcClient::GenericReadCb(std::shared_ptr<network::Connection> c, 
                              libbase::ByteBuffer& data, 
                              const ReadAction& action, int id) {
  LOG_INFO("Server Response: %s", data.ToString().c_str());
  const char* end_pos = data.Find("#", data.AddrOfRead());
  if (end_pos) {
    // using endian conversion maybe better
    int len = std::stoi(std::string(data.AddrOfRead(), end_pos - data.AddrOfRead()));
    if (len < 0) {
      LOG_ERROR("Invalid data length = %d", len);
      Close(id);
    }
    int data_len = data.ReadableBytes() - (end_pos - data.AddrOfRead()) - 1;
    if (data_len >= len) {
      data.MoveReadPos(end_pos - data.AddrOfRead() + 1);
      action(data);
      Close(id);
    }
  }
}

void RpcClient::GenericTimeoutCallback(int id) {
  ReqIter it = requests_.find(id);
  if (it == requests_.end()) return;
  Delete(it);
}

void RpcClient::Register(int id, libbase::ByteBuffer& buf, 
                         const ReadAction& action, int timeout, bool async) {
  if (requests_.count(id)) {
    LOG_WARN("Request Id: %d is already exist!", id);
    return;
  }
  RequestContext ctx;
  ctx.request_id = id;
  int sockfd = network::Connector::GetConnectedSocket(1, server_addr_, 1);
  ctx.conn = std::shared_ptr<network::Connection>(
             new network::Connection(sockfd, loop_.EpollPtr()));

  if (timeout > 0) {
    ctx.timer = network::Timer(std::bind(&RpcClient::GenericTimeoutCallback, 
                               this, id), timeout);
    loop_.EpollPtr()->RunTimer(&(ctx.timer_id), ctx.timer);
  }
  ctx.async = async;
  requests_[id] = ctx;
  (ctx.conn)->SetReadOperation(std::bind(&RpcClient::GenericReadCb, 
                               this, std::placeholders::_1, 
                               std::placeholders::_2, action, id));
  (ctx.conn)->SetInitOperation(std::bind(&RpcClient::GenericInitCb, 
                               this, std::placeholders::_1, buf));
  (ctx.conn)->SetCloseOperation(std::bind(&RpcClient::Close, this, id));
  (ctx.conn)->Initialize();
  if (!async) Pause();
}

void RpcClient::Close(int id) {
  ReqIter it = requests_.find(id);
  if (it == requests_.end()) {
    LOG_WARN("Close unexist request: #%d", id);
    return;
  }
  Delete(it);
}

void RpcClient::Delete(ReqIter it) {
  if ((it->second).async == false) Resume();
  std::shared_ptr<network::Connection> backup = (it->second).conn;
  (it->second).conn.reset();
  loop_.EpollPtr()->RunLater(std::bind(&RpcClient::DeleteInLoop, this, backup));
  requests_.erase(it);
}

void RpcClient::DeleteInLoop(std::shared_ptr<network::Connection> c) {
  c->DestroyedInLoop(c);
}

} // namespace leetrpc
