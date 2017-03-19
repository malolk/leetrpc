#include "rpc_status.h"

#include <assert.h>
#include <string.h>

namespace leetrpc {

RpcStatus::Status* RpcStatus::CopyStatus(Status* s) {
  status_ = reinterpret_cast<Status*>(new char[sizeof(*s) + s->len]);
  memcpy(status_, s, sizeof(*s));
  memcpy(status_->msg, s->msg, s->len);
}

void RpcStatus::MsgToStatus(Code c, const char* msg, int len) {
  assert(msg);
  delete[] status_;
  status_ = reinterpret_cast<Status*>(new char[sizeof(Status) + len]);
  status_->code = c;
  status_->len = len;
  memcpy(status_->msg, msg, len);
}

std::string RpcStatus::ToString() const {
  std::string ret;
  if (status_ == NULL) return "OK";
  switch (status_->code) {
    case kPARSE_ERROR: ret = "Parse Error "; break;
    case kINVALID_REQUEST: ret = "Invalide Request "; break;
    case kNOT_FOUND_METHOD: ret = "No Such Method "; break;
    case kINTERNAL_ERROR: ret = "Internal Error "; break;
    case kTIMEOUT: ret = "Time out "; break;
    default: ret = std::string("Unknown state: ") 
                 + std::to_string(status_->code) ; break;
  }
  if (status_->len > 0) {
    ret.append(": ", 2);
    ret.append(status_->msg, status_->len);
  }
  return ret;
}

} // namespace leetrpc
