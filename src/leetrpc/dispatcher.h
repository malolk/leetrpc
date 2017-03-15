#ifndef LEETRPC_DISPATCHER_H__
#define LEETRPC_DISPATCHER_H__

#include "jsonutil/json.h"

namespace leetrpc {
class Dispatcher {
 public:
  virtual ~Dispatcher() { }
  virtual int ServiceId() const = 0;
  virtual void Dispatch(const jsonutil::Value& request,
                        jsonutil::Value& response, int method_id) = 0;
};
} // namespace leetrpc
#endif // LEETRPC_DISPATCHER_H__
