#include "leetrpc/rpc_client.h"
#include "libbase/loggerutil.h"

#include <stdlib.h>

namespace leetrpc {

void DefaultErrorCb(const RpcStatus& st) {
  LOG_ERROR("%s", st.ToString().c_str());
}

void RpcClient::GenericInitCb(ConnPtr c, libbase::ByteBuffer& data) {
  libbase::ByteBuffer length;
  length.AppendString(std::to_string(data.ReadableBytes()) + "#");
  c->Send(length);
  c->Send(data);
  data.Reset();
}

void RpcClient::GenericReadCb(ConnPtr c, libbase::ByteBuffer& data, 
                              const ReadAction& action, int id) {
  LOG_INFO("Server Response: %s", data.ToString().c_str());
  const char* end_pos = data.Find("#", data.AddrOfRead());
  if (end_pos) {
    // using endian conversion maybe better
    int len = std::stoi(std::string(data.AddrOfRead(), end_pos - data.AddrOfRead()));
    if (len < 0) {
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
  LOG_INFO("%s", "Time out");
  if (it == requests_.end()) return;
  ConnPtr c = (it->second).conn;
  c->SetTimeout(); // connection is timeout.
}

void RpcClient::Register(int id, libbase::ByteBuffer& buf, 
                         const ReadAction& action, int timeout, bool async) {
  if (requests_.count(id)) {
    LOG_WARN("Request Id: %d is already exist!", id);
    return;
  }
  int sockfd = network::Connector::GetConnectedSocket(1, server_addr_, 1);
  ConnPtr conn(new network::Connection(sockfd, loop_.EpollPtr()));
  conn->SetReadOperation(std::bind(&RpcClient::GenericReadCb, 
                         this, std::placeholders::_1, 
                         std::placeholders::_2, action, id));
  conn->SetInitOperation(std::bind(&RpcClient::GenericInitCb, 
                         this, std::placeholders::_1, buf));
  conn->SetCloseOperation(std::bind(&RpcClient::Close, this, id));
  AddRequestContext(conn, timeout, id, async);
}

void RpcClient::AddRequestContext(ConnPtr conn, int timeout, int id, bool async) {
  loop_.EpollPtr()->RunLater(std::bind(&RpcClient::AddRequestContextInLoop, 
                                       this, conn, timeout, id, async));
  if (!async) Pause();
}

void RpcClient::AddRequestContextInLoop(ConnPtr conn, int timeout, int id, bool async) {
  loop_.EpollPtr()->MustInLoopThread();
  requests_[id] = {conn, network::TimerQueue::TimerIdType(), id, async};
  conn->Initialize();
  if (timeout > 0) {
    network::Timer timer = network::Timer(std::bind(&RpcClient::GenericTimeoutCallback, this, id), timeout);
    loop_.EpollPtr()->RunTimer(&(requests_[id].timer_id), timer);
  }
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
  if (!(it->second).async) Resume();
  ConnPtr backup = (it->second).conn;
  (it->second).conn.reset();
  loop_.EpollPtr()->RunLater(std::bind(&RpcClient::DeleteInLoop, this, backup));
  requests_.erase(it);
}

void RpcClient::DeleteInLoop(ConnPtr c) {
  c->DestroyedInLoop(c);
}

void RpcClient::Pause() {
  assert(is_paused_ == false);
  is_paused_ = true;
  libbase::MutexLockGuard m(mu_);
  while (is_paused_) {
    condvar_.Wait();
  }
}

void RpcClient::Resume() {
  libbase::MutexLockGuard m(mu_);
  is_paused_ = false;
  condvar_.NotifyOne();
}

} // namespace leetrpc
