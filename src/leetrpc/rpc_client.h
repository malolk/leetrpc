#ifndef LEETRPC_RPC_CLIENT_H__
#define LEETRPC_RPC_CLIENT_H__

#include "network/epoll.h"
#include "network/eventloop.h"
#include "network/socket_address.h"
#include "network/connection.h"
#include "network/connector.h"
#include "network/timer_queue.h"
#include "network/timer.h"
#include "libbase/buffer.h"
#include "libbase/noncopyable.h"
#include "libbase/mutexlock.h"
#include "libbase/condvar.h"
#include "jsonutil/json.h"

#include <memory>
#include <functional>
#include <atomic>
#include <unordered_map>

namespace leetrpc {

typedef struct {
  std::shared_ptr<network::Connection> conn;
  network::TimerQueue::TimerIdType timer_id;
  network::Timer timer;
  int request_id;
  bool async;
} RequestContext;



class RpcClient: private libbase::Noncopyable {
 public:
  typedef std::function<void(libbase::ByteBuffer& )> ReadAction;
  typedef network::Connection::CallbackInitType InitCb;
  typedef network::Connection::CallbackReadType ReadCb;

  RpcClient(const std::string& ip, uint16_t port): 
    server_addr_(network::SocketAddress(ip, port)), 
    next_id_(0), loop_(), mu_(), condvar_(mu_) {
    loop_.Start();
  }
  
  ~RpcClient() {
    loop_.Stop();
    loop_.Join();
  }
  
  void Register(int id, libbase::ByteBuffer& buf, 
                const ReadAction& action, int timeout = -1, bool async = false);
  void Close(int id);
  /* TODO: Is GenId() atomic ? */
  int GenId() { return ++next_id_; }
  void Pause() {
    libbase::MutexLockGuard m(mu_);
    condvar_.Wait();
  }
  void Resume() {
    libbase::MutexLockGuard m(mu_);
    condvar_.NotifyOne();
  }

 private:
  void GenericTimeoutCallback(int req_id);
  void GenericInitCb(std::shared_ptr<network::Connection> c, libbase::ByteBuffer& data);
  void GenericReadCb(std::shared_ptr<network::Connection> c, 
                     libbase::ByteBuffer& buf, const ReadAction& action, int id);

  typedef std::unordered_map<int, RequestContext>::iterator ReqIter;
  void Delete(ReqIter it);
  void DeleteInLoop(std::shared_ptr<network::Connection> c);
  network::SocketAddress server_addr_;

  std::unordered_map<int, RequestContext> requests_;
  std::atomic<int> next_id_; // for request
  network::EventLoop loop_;
  libbase::MutexLock mu_; // used to pause client
  libbase::CondVar condvar_; // used to pause client
};

template <typename T>
void GenericActionCb(libbase::ByteBuffer& buf, T* ret) {
  jsonutil::Value response;
  response.Parse(buf.AddrOfRead(), buf.ReadableBytes());
  *(response.GetArrayValue(1)) >> (*ret);
}

} // namespace leetrpc
#endif // LEETRPC_RPC_CLIENT_H__
