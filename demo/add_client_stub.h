#ifndef LEETRPC_ADD_CLIENT_H__
#define LEETRPC_ADD_CLIENT_H__

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
class AddClientStub {
 public:
  AddClientStub(RpcClient& c): c_(c) {
  }

  double Add(double a, double b, RpcStatus* rsp = NULL, int timeout = -1) {
    libbase::ByteBuffer req_buf;
    int req_id = AddCommon(a, b, req_buf);
    double res;
    c_.Register(req_id, req_buf, std::bind(&GenericActionCb<double>, std::placeholders::_1, &res, rsp), timeout);
    return res;
  }

  void Add(double a, double b, AsynActionCb<double> asyn_handler, int timeout = -1, ErrorActionCb error_cb = DefaultErrorCb) {
    libbase::ByteBuffer req_buf;
    int req_id = AddCommon(a, b, req_buf);
    c_.Register(req_id, req_buf, std::bind(&GenericAsynActionCb<double>, std::placeholders::_1, asyn_handler, error_cb), timeout, true);
  }

 private:
  int AddCommon(double a, double b, libbase::ByteBuffer& req_buf) {
    jsonutil::Value request(jsonutil::kJSON_ARRAY);
    jsonutil::Builder<jsonutil::Value> batch;
    int req_id = c_.GenId();;
    batch << req_id << 0 << 0;
    jsonutil::Value params(jsonutil::kJSON_ARRAY);
    jsonutil::Builder<jsonutil::Value> arg_batch;
    arg_batch << a << b;
    params.MergeArrayBuilder(arg_batch);
    batch << params;
    request.MergeArrayBuilder(batch);
    req_buf.AppendString(request.ToString());
    return req_id;
  } 

  RpcClient& c_;
};
}
#endif
