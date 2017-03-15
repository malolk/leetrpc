#ifndef LEETRPC_ADD_SERVER_STUB_H__
#define LEETRPC_ADD_SERVER_STUB_H__

#include "leetrpc/dispatcher.h"
#include "libbase/buffer.h"
#include "jsonutil/json.h"

namespace leetrpc {
class AddServerStub: public Dispatcher {
 public:
  AddServerStub(): service_id_(0) {
  }

  virtual double Add(double a, double b) = 0;

  void Dispatch(const jsonutil::Value& request, 
                jsonutil::Value& response, int method_id) {
    LOG_INFO("%s", request.ToString().c_str());
    if (method_id == 0) {
      jsonutil::Builder<jsonutil::Value> batch;
      batch << *(request.GetArrayValue(0));
      const jsonutil::Value* params = request.GetArrayValue(3);
      double a, b;
      *(params->GetArrayValue(0)) >> a;
      *(params->GetArrayValue(1)) >> b;
      double res = Add(a, b);
      batch << res;
      response.MergeArrayBuilder(batch);
    }
  }

  int ServiceId() const {
    return service_id_;
  }

 private:
  int service_id_;  
};
} // namespace leetrpc
#endif // LEETRPC_ADD_SERVER_STUB_H__
