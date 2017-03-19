#include "leetrpc/rpc_server.h"
#include "leetrpc/rpc_status.h"
#include "jsonutil/json.h"

#include <unistd.h>

/* Request frame:  <Request Id> <Service Id> <Method Id> <params>
 * Response frame: <Request Id> <Status Code> <Return-Value | Error Message>*/

namespace leetrpc {
void RpcServer::ReadCb(Conn c, Buf& data) {
  const char* end_pos = data.Find("#", data.AddrOfRead());
  // TODO: handle multiple requests via one connection
  if (end_pos) {
    int prefix_len = end_pos - data.AddrOfRead();
    int len = std::stoi(std::string(data.AddrOfRead(), prefix_len));
    int data_len = data.ReadableBytes() - prefix_len - 1;
    if (data_len >= len) {
      sleep(3);
      LOG_INFO("Request Frame: %s", data.ToString().c_str());
      data.MoveReadPos(prefix_len + 1); // +1 to skip '#'
      jsonutil::Value request, response(jsonutil::kJSON_ARRAY);
      jsonutil::JsonStatus st = request.Parse(data.AddrOfRead(), len);
      if (!st.Ok()) {
        SendErrorResponse(c, -1, RpcStatus::kPARSE_ERROR, st.ToString());                
        return;
      }
      data.MoveReadPos(len);
      double s_id, m_id;
      *(request.GetArrayValue(1)) >> s_id;
      if (context_.empty() || s_id < 0 || s_id >= context_.size()) {
        SendErrorResponse(c, -1, RpcStatus::kINVALID_REQUEST, "No such service");          
        return;
      }
      *(request.GetArrayValue(2)) >> m_id;
      context_[s_id]->Dispatch(request, response, m_id);
      SendWithPrefixLen(c, response);
    }
  }
}

void RpcServer::SendErrorResponse(Conn c, int req_id, RpcStatus::Code code, 
                                  const std::string& msg) {
  jsonutil::Value response(jsonutil::kJSON_ARRAY);
  jsonutil::Builder<jsonutil::Value> batch;
  batch << req_id << code << msg;
  response.MergeArrayBuilder(batch);
  SendWithPrefixLen(c, response);
}

void RpcServer::SendWithPrefixLen(Conn c, const jsonutil::Value& response) {
  std::string reply = response.ToString();
  libbase::ByteBuffer reply_bulk;
  reply_bulk.AppendString(std::to_string(reply.size()) + "#" + reply);
  c->Send(reply_bulk);
}

void RpcServer::Register(Dispatcher* d) {
  assert(d);
  int id = d->ServiceId();
  if (context_.count(id)) {
    LOG_WARN("Service #%d is already exist", id);
    return;
  }
  context_[id] = d;
}

RpcServer::~RpcServer() {
  for (std::unordered_map<int, Dispatcher*>::iterator 
       it = context_.begin(); it != context_.end(); ++it) {
    delete(it->second); 
  }    
}

} // namespace leetrpc
