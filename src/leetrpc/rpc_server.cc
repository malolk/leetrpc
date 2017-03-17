#include "leetrpc/rpc_server.h"
#include "jsonutil/json.h"

#include <unistd.h>

namespace leetrpc {
void RpcServer::ReadCb(std::shared_ptr<network::Connection> c,
                       libbase::ByteBuffer& data) {
  LOG_INFO("Client Request Raw Data: %s", data.ToString().c_str());
  const char* end_pos = data.Find("#", data.AddrOfRead());
  // TODO: handle multiple requests via one connection
  if (end_pos) {
    int len = std::stoi(std::string(data.AddrOfRead(), end_pos - data.AddrOfRead()));
    if (len <= 0) {
      LOG_ERROR("Invalid data length = %d", len);
      // TODO: Handle invalid data length
    }
    int data_len = data.ReadableBytes() - (end_pos - data.AddrOfRead()) - 1;
    if (data_len >= len) {
      sleep(3);
      LOG_INFO("Client Request Raw Data: %s", data.ToString().c_str());
      data.MoveReadPos(end_pos - data.AddrOfRead() + 1);
      jsonutil::Value request, response(jsonutil::kJSON_ARRAY);
      // TODO: Handle parser status
      LOG_INFO("Client Request: %s", data.ToString().c_str());
      LOG_INFO("Client Request: length = %d", len);
      LOG_INFO("Client Request: %s", std::string(data.AddrOfRead(), len).c_str());

      int status = request.Parse(data.AddrOfRead(), len);
      LOG_INFO("STATUS: %d", status);
      LOG_INFO("Client Request: %s", request.ToString().c_str());
      data.MoveReadPos(len);
      double s_id, m_id;
      *(request.GetArrayValue(1)) >> s_id;
      *(request.GetArrayValue(2)) >> m_id;
      context_[s_id]->Dispatch(request, response, m_id);
      std::string reply = response.ToString();
      LOG_INFO("Server Response: %s", reply.c_str());
      libbase::ByteBuffer reply_bulk;
      reply_bulk.AppendString(std::to_string(reply.size()) + "#" + reply);
      c->Send(reply_bulk);
    }
  }
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
