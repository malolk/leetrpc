#ifndef LEETRPC_ADD_CLIENT_H__
#define LEETRPC_ADD_CLIENT_H__

#include "jsonutil/json.h"
#include "leetrpc/rpc_client.h"
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

  double Add(double a, double b)  { 
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
    libbase::ByteBuffer req_buf;
    req_buf.AppendString(request.ToString());
    double res;
    c_.Register(req_id, req_buf, std::bind(&GenericActionCb<double>, std::placeholders::_1, &res), -1);
    return res;
  } 

  std::vector<double> Sum(std::vector<double> a, double num)  { 
    jsonutil::Value request(jsonutil::kJSON_ARRAY);
    jsonutil::Builder<jsonutil::Value> batch;
    int req_id = c_.GenId();;
    batch << req_id << 0 << 1;
    jsonutil::Value params(jsonutil::kJSON_ARRAY);
    jsonutil::Builder<jsonutil::Value> arg_batch;
    arg_batch << a << num;
    params.MergeArrayBuilder(arg_batch);
    batch << params;
    request.MergeArrayBuilder(batch);
    libbase::ByteBuffer req_buf;
    req_buf.AppendString(request.ToString());
    std::vector<double> res;
    c_.Register(req_id, req_buf, std::bind(&GenericActionCb<std::vector<double>>, std::placeholders::_1, &res), -1);
    return res;
  } 

 private:
  RpcClient& c_;
};
}
#endif
