#ifndef LEETRPC_RPC_STATUS_H__
#define LEETRPC_RPC_STATUS_H__

#include <string>

namespace leetrpc {
class RpcStatus {
 public:
  typedef enum {
    kOK = 0,
    kINVALID_CODE = -1,
    kPARSE_ERROR = -32700,
    kINVALID_REQUEST = -32600,
    kNOT_FOUND_METHOD = -32601,
    kINVALID_PARAMS = -32602,
    kINTERNAL_ERROR = -32603,
    kTIMEOUT = -32604
  } Code;

  RpcStatus(): status_(NULL) { }
  ~RpcStatus() {
    delete[] status_;
  }
  
  RpcStatus(const RpcStatus& rhs) { 
    status_ = rhs.status_ ? CopyStatus(rhs.status_) : NULL;
  }

  void operator=(const RpcStatus& rhs) { 
    if (status_ != rhs.status_) {
      delete[] status_;
      status_ = rhs.status_ ? CopyStatus(rhs.status_) : NULL;
    }
  }

  void ParseError(const char* msg, int len) {
    MsgToStatus(kPARSE_ERROR, msg, len);
  }

  void InvalidRequest(const char* msg, int len) {
    MsgToStatus(kINVALID_REQUEST, msg, len);
  }

  void NotFoundMethod(const char* msg, int len) {
    MsgToStatus(kNOT_FOUND_METHOD, msg, len);
  }

  void InvalidParams(const char* msg, int len) {
    MsgToStatus(kINVALID_PARAMS, msg, len);
  }

  void InternalError(const char* msg, int len) {
    MsgToStatus(kINTERNAL_ERROR, msg, len);
  }

  void TimeoutNotify() {
    MsgToStatus(kTIMEOUT, "", 0);
  }

  void BeOK() {
    delete[] status_;
    status_ = NULL;
  }

  void MsgToStatus(Code c, const char* msg, int len);
  
  bool Ok() const { return status_ == NULL; }
  std::string ToString () const;
  
  Code CodeNum() { return status_ == NULL ? kOK : status_->code; }
 private:
  typedef struct {
    Code code;
    int len;
    char msg[0];
  } Status;

  Status* CopyStatus(Status* s);
  Status* status_;
};

} // namespace leetrpc

#endif // LEETRPC_RPC_STATUS_H__

