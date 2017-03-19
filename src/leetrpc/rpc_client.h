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
#include "jsonutil/json_status.h"
#include "leetrpc/rpc_status.h"

#include <memory>
#include <functional>
#include <atomic>
#include <unordered_map>

namespace leetrpc {

typedef std::shared_ptr<network::Connection> ConnPtr;

typedef struct {
  ConnPtr conn;
  network::TimerQueue::TimerIdType timer_id;
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
    next_id_(0), loop_(), is_paused_(false), mu_(), condvar_(mu_) {
    loop_.Start();
  }
  
  ~RpcClient() {
    loop_.Stop();
    loop_.Join();
  }
  
  void Register(int id, libbase::ByteBuffer& buf, 
                const ReadAction& action, int timeout = -1, 
                bool async = false);
  void Close(int id);
  /* TODO: Is GenId() atomic ? */
  int GenId() { return ++next_id_; }
  void Pause();
  void Resume();

 private:
  void AddRequestContext(ConnPtr conn, int timeout, int id, bool async);
  void AddRequestContextInLoop(ConnPtr conn, int timeout, int id, bool async);
  void GenericTimeoutCallback(int req_id);
  void GenericInitCb(ConnPtr c, libbase::ByteBuffer& data);
  void GenericReadCb(ConnPtr c, libbase::ByteBuffer& buf, const ReadAction& action, int id);

  typedef std::unordered_map<int, RequestContext>::iterator ReqIter;
  void Delete(ReqIter it);
  void DeleteInLoop(ConnPtr c);
  network::SocketAddress server_addr_;

  std::unordered_map<int, RequestContext> requests_;
  std::atomic<int> next_id_; // for request
  network::EventLoop loop_;
  std::atomic<bool> is_paused_; 
  libbase::MutexLock mu_; // used to pause client
  libbase::CondVar condvar_; // used to pause client
};


template <typename T>
void GenericActionCb(libbase::ByteBuffer& buf, T* ret, RpcStatus* rsp) {
  LOG_INFO("Response: %s", buf.ToString().c_str());
  int len = buf.ReadableBytes();
  jsonutil::Value response;
  jsonutil::JsonStatus js = response.Parse(buf.AddrOfRead(), len);

  if (js.Ok()) {
    double code = (response.GetArrayValue(1))->GetNumber();
    if (code == RpcStatus::kOK) { 
      *(response.GetArrayValue(2)) >> (*ret);
      if (rsp) rsp->BeOK();
    } else {
      if (rsp) {
        jsonutil::Value* p = response.GetArrayValue(2);
        rsp->MsgToStatus(static_cast<RpcStatus::Code>(code), 
                         p->GetString(), p->GetStringLength());
      }
    }
  } else if (rsp) {
    rsp->ParseError(buf.ToString().c_str(), len); 
  }
}

template <typename T>
using AsynActionCb = std::function<void(std::shared_ptr<T>)>;
using ErrorActionCb = std::function<void(RpcStatus&)>;

void DefaultErrorCb(const RpcStatus& st);

template <typename T>
void GenericAsynActionCb(libbase::ByteBuffer& buf, 
                         AsynActionCb<T> asyn_handler, 
                         ErrorActionCb error_cb) {
  LOG_INFO("Response: %s", buf.ToString().c_str());
  int len = buf.ReadableBytes();
  RpcStatus rs;
  jsonutil::Value response;
  jsonutil::JsonStatus js = response.Parse(buf.AddrOfRead(), len);
  if (js.Ok()) {
    double code = (response.GetArrayValue(1))->GetNumber();
    if (code == RpcStatus::kOK) {
      std::shared_ptr<T> ptr(new T());
      *(response.GetArrayValue(2)) >> (*ptr);
      // TODO: shift asyn_handler to a working thread.
      asyn_handler(ptr);
    } else {
      jsonutil::Value* p = response.GetArrayValue(2);
      rs.MsgToStatus(static_cast<RpcStatus::Code>(code), 
                     p->GetString(), p->GetStringLength());
      error_cb(rs);
    }
  } else {
    rs.ParseError(buf.ToString().c_str(), len);  
    error_cb(rs);
  }
}

} // namespace leetrpc

#endif // LEETRPC_RPC_CLIENT_H__

