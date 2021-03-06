#ifndef LEETRPC_ADD_SERVER_H__
#define LEETRPC_ADD_SERVER_H__

#include "jsonutil/json.h"
#include "leetrpc/dispatcher.h"
#include "leetrpc/rpc_client.h"
#include "leetrpc/rpc_status.h"
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <functional>

namespace leetrpc {
class AddServerStub: public Dispatcher {
 public:
  AddServerStub(): service_id_(0) {
  }

  virtual double Add(double a, double b) = 0;

  void Dispatch(const jsonutil::Value& req, jsonutil::Value& response, int m_id) {
    jsonutil::Builder<jsonutil::Value> batch;
    const jsonutil::Value* params = req.GetArrayValue(3);
    RpcStatus st;
    if (m_id == 0) {
      batch << *(req.GetArrayValue(0));
      double a;
      *(params->GetArrayValue(0)) >> a;
      double b;
      *(params->GetArrayValue(1)) >> b;
      double res = Add(a, b);
      batch << st.CodeNum() << res;
    } else {
      std::string m_id_s = std::to_string(m_id);
      st.NotFoundMethod(m_id_s.c_str(), m_id_s.size());
      batch << *(req.GetArrayValue(0)) << st.CodeNum() << st.ToString();
    }

    response.MergeArrayBuilder(batch);
  }

  int ServiceId() const {
    return service_id_;
  }

 private:
  int service_id_;
};
}
#endif
