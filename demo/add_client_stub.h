#ifndef LEETRPC_ADD_CLIENT_STUB_H__
#define LEETRPC_ADD_CLIENT_STUB_H__

#include "jsonutil/json.h"
#include "leetrpc/rpc_client.h"
#include "libbase/buffer.h"
#include "libbase/loggerutil.h"

#include <string>

namespace leetrpc {
class AddClientStub {
 public:
  AddClientStub(RpcClient& c): c_(c) {
  }

  // frame: [req_id] [s_id] [m_id] [params]
  double Add(double a, double b) {
    jsonutil::Value request(jsonutil::kJSON_ARRAY);
    double req_id = c_.GenId();
    double s_id = 0, m_id = 0;
    jsonutil::Builder<jsonutil::Value> batch;
    batch << req_id << s_id << m_id;
    jsonutil::Value params(jsonutil::kJSON_ARRAY);
    jsonutil::Builder<jsonutil::Value> arg_batch;
    arg_batch << a << b;
    params.MergeArrayBuilder(arg_batch);
    batch << params;
    request.MergeArrayBuilder(batch);
    libbase::ByteBuffer buf;
    std::string reqs = request.ToString();
    LOG_INFO("Request: %s", reqs.c_str());
    buf.AppendString(reqs);
    double res = 0;
    c_.Register(req_id, buf, 
      std::bind(&AddClientStub::AddCb, 
                std::placeholders::_1, &res), -1);
    return res;
  }

  static void AddCb(libbase::ByteBuffer& buf, double* res) {
    jsonutil::Value response;
    response.Parse(buf.AddrOfRead(), buf.ReadableBytes());
    LOG_INFO("Response: %s", response.ToString().c_str());
    *(response.GetArrayValue(1)) >> (*res);
  }

 private:
  RpcClient& c_;
};

} // namespace leetrpc
#endif // LEETRPC_ADD_CLIENT_STUB_H__
