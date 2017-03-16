#include "context.h"
#include "ast.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#include <string>
#include <algorithm>
#include <iostream>

namespace yy {

std::string Context::GenerateClassName(const std::string& cls, const std::string& name) {
  return cls + name + "Stub";
}

std::string Context::AddClassHeader(const std::string& cls, const std::string& extends, const std::string& name) {
  return std::string("class ") 
         + GenerateClassName(cls, name) 
         + extends + " {\r\n public:\r\n";
}

std::string Context::AddClassFooter() {
  return "};\r\n";
}

static std::string TransferStdType(const std::string& type) {
  if (type.find("vector") != std::string::npos 
      || type.find("string") != std::string::npos
      || type.find("map") != std::string::npos) {
    return std::string("std::") + type;
  } 
  return type;
}

static std::string AddArgImpl(NodeArg* a, const std::string& d, int add_type) {
  std::string buf;
  while (a) {
    if (add_type) {
      // add_type: 0 without type, 1 add type
      std::string arg_t = a->arg_t;
      buf += TransferStdType(a->arg_t) + " ";
    }
    buf += a->name;
    a = a->next;
    if (a) buf += d + " ";
  }
  return buf;
}

std::string Context::AddArg(NodeArg* a) {
  return AddArgImpl(a, ",", 1);
}

std::string Context::AddFuncSignature(NodeFunc* f) {
  std::string buf;
  buf += TransferStdType(f->ret_t) + " " + f->name 
         + "(" + AddArg(reinterpret_cast<NodeArg*>(f->arg)) 
         + ")";
  return buf;
}

std::string Context::AddIndentation(int s) {
  return std::string(s, ' ');
}

std::string Context::AddServerFunc(NodeFunc* f, int s_id, int f_id) {
  std::string buf;
  buf += AddIndentation(2);
  buf += AddFuncSignature(f) + ";\r\n";
  return buf;
}

std::string Context::AddOneIncluder(const std::string& header, int is_std) {
  std::string buf;
  buf = buf + "#include " + (is_std ? "<" : "\"") 
        + header + (is_std ? ">" : "\"") + "\r\n"; 
  return buf;
}

/* Request frame: [req_id] [service_id] [method_id] [params] */
std::string Context::AddClientFunc(NodeFunc* f, int s_id, int f_id) {
  std::string buf;
  std::string ss_id = std::to_string(s_id);
  std::string sf_id = std::to_string(f_id);
  buf += AddIndentation(2) + AddFuncSignature(f);
  buf += AddIndentation(2) +  "{"                                              + " \r\n";
  buf += AddIndentation(4) +  "jsonutil::Value request(jsonutil::kJSON_ARRAY)" + ";\r\n";
  buf += AddIndentation(4) +  "jsonutil::Builder<jsonutil::Value> batch"       + ";\r\n";
  buf += AddIndentation(4) +  "int req_id = c_.GenId();"                       + ";\r\n";
  buf += AddIndentation(4) +  "batch << req_id << " + ss_id + " << " + sf_id   + ";\r\n";
  buf += AddIndentation(4) +  "jsonutil::Value params(jsonutil::kJSON_ARRAY)"  + ";\r\n";
  buf += AddIndentation(4) +  "jsonutil::Builder<jsonutil::Value> arg_batch"   + ";\r\n";
  buf += AddIndentation(4) +  "arg_batch << " 
                           +  AddArgImpl(
                              reinterpret_cast<NodeArg*>(f->arg), " <<", 0)    + ";\r\n";
  buf += AddIndentation(4) +  "params.MergeArrayBuilder(arg_batch)"            + ";\r\n";
  buf += AddIndentation(4) +  "batch << params"                                + ";\r\n";
  buf += AddIndentation(4) +  "request.MergeArrayBuilder(batch)"               + ";\r\n";
  buf += AddIndentation(4) +  "libbase::ByteBuffer req_buf"                    + ";\r\n";
  buf += AddIndentation(4) +  "req_buf.AppendString(request.ToString())"       + ";\r\n";
  buf += AddIndentation(4) +  TransferStdType(f->ret_t) + " res"               + ";\r\n";
  buf += AddIndentation(4) +  "c_.Register(req_id, req_buf," 
                           +  " std::bind(&GenericActionCb<" 
                           +  TransferStdType(f->ret_t) 
                           +  ">, std::placeholders::_1, &res), -1)"           + ";\r\n";
  buf += AddIndentation(4) +  "return res"                                     + ";\r\n";
  buf += AddIndentation(2) +  "}"                                              + " \r\n";
  return buf;
}

/* Request frame: [req_id] [service_id] [method_id] [params] */
std::string Context::AddServerFunc(NodeFunc* f) {
  return (AddIndentation(2) + "virtual " +  AddFuncSignature(f) + " = 0;\r\n\r\n");
}

std::string AddDispatchSignature() {
  return "void Dispatch(const jsonutil::Value& req, jsonutil::Value& response, int m_id)";
}

std::string Context::AddServerOneMethodDispatcher(const NodeFunc* f) {
  std::string buf;
  const NodeArg* arg = reinterpret_cast<NodeArg*>(f->arg);
  buf += AddIndentation(6) + "batch << *(req.GetArrayValue(0))" + ";\r\n";
  std::string calls = TransferStdType(f->ret_t) + " res = " + f->name + "(";
  for (int i = 0; arg != NULL; ++i, arg = arg->next) {
    buf += AddIndentation(6) + TransferStdType(arg->arg_t) + " " + arg->name + ";\r\n";
    buf += AddIndentation(6) + "*(params->GetArrayValue(" + std::to_string(i) + ")) >> " + arg->name + ";\r\n";
    if (arg->next) calls = calls + arg->name + ", ";
    else calls = calls + arg->name;
  }
  buf += AddIndentation(6) + calls + ");\r\n";
  buf += AddIndentation(6) + "batch << res" + ";\r\n";
  return buf;
}

std::string Context::AddServerMethodDispatcher(const std::vector<NodeFunc*>& funcs) {
  std::string buf;
  int size = static_cast<int>(funcs.size());
  buf += AddIndentation(4);
  for (int i = 0; i < size; ++i) {
    buf  = buf + "if (m_id == " + std::to_string(i) + ") {\r\n";
    buf += AddServerOneMethodDispatcher(funcs[i]);
    buf += AddIndentation(4);
    if (i == (size - 1)) buf += "}\r\n";
    else buf += "} else ";
  }
  return buf;
}

std::string Context::AddServerDispatcher(const std::vector<NodeFunc*>& funcs) {
  std::string buf;
  buf += AddIndentation(2) + AddDispatchSignature() + " {\r\n";
  buf += AddIndentation(4) + "jsonutil::Builder<jsonutil::Value> batch" + ";\r\n";
  buf += AddIndentation(4) + "const jsonutil::Value* params = req.GetArrayValue(3)" + ";\r\n";
  buf += AddServerMethodDispatcher(funcs);
  buf += AddIndentation(4) + "response.MergeArrayBuilder(batch)" + ";\r\n";
  buf += AddIndentation(2) + "}\r\n\r\n";
  return buf;
}

std::string Context::AddServerServiceId() {
  std::string buf;
  buf += AddIndentation(2) + "int ServiceId() const {\r\n";
  buf += AddIndentation(4) + "return service_id_;\r\n";
  buf += AddIndentation(2) + "}\r\n\r\n";
  return buf;
}

std::string Context::AddServerData() {
  std::string buf;
  buf += AddIndentation(1) + "private:\r\n";
  buf += AddIndentation(2) + "int service_id_;\r\n";
  return buf;
}

std::string Context::AddServerConstructor(const std::string& cls, const std::string& port, int service_id) {
  std::string buf;
  buf += AddIndentation(2) + GenerateClassName(cls, port)
      + "(): service_id_(" + std::to_string(service_id) + ") {\r\n";
  buf += AddIndentation(2) + "}\r\n\r\n";
  return buf;
}

std::string Context::AddClientData() {
  std::string buf;
  buf += AddIndentation(1) + "private:\r\n";
  buf += AddIndentation(2) + "RpcClient& c_;\r\n";
  return buf;
}

std::string Context::AddIncluder() {
  std::string buf;
  buf += AddOneIncluder("jsonutil/json.h", 0);
  buf += AddOneIncluder("leetrpc/rpc_client.h", 0);
  buf += AddOneIncluder("map", 1);
  buf += AddOneIncluder("string", 1);
  buf += AddOneIncluder("vector", 1);
  buf += AddOneIncluder("utility", 1);
  buf += AddOneIncluder("functional", 1);
  buf += "\r\n";
  return buf;
}

std::string Context::AddHeaderGuardStart(std::string name, std::string port) {
  std::string buf;
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);
  std::transform(port.begin(), port.end(), port.begin(), ::toupper);
  std::string def_name = std::string("LEETRPC_") + name + "_" + port + "_H__";
  buf = buf + "#ifndef " + def_name + "\r\n";
  buf = buf + "#define " + def_name + "\r\n\r\n";
  return buf;
}

std::string Context::AddHeaderGuardEnd() {
  return "#endif\r\n";
}

std::string Context::AddNameSpaceStart(const std::string& name) {
  return std::string("namespace ") + name +  " {\r\n";
}

std::string Context::AddNameSpaceEnd() {
  return std::string("}\r\n");
}

std::string Context::AddClientConstructor(const std::string& cls, const std::string& port) {
  std::string buf;
  buf += AddIndentation(2) + GenerateClassName(cls, port) 
         + "(RpcClient& c): c_(c) {\r\n";
  buf += AddIndentation(2) + "}\r\n\r\n";
  return buf;
}

void DumpToFile(const std::string& pathname, const std::string& data) {
  umask(0);
  int fd = open(pathname.c_str(), O_WRONLY | O_CREAT, 0666);
  if (fd < 0) {
    fprintf(stderr, "file [%s] create failed!", pathname.c_str());
    return;
  }
  ssize_t num = ::write(fd, data.c_str(), data.size());
  ::close(fd);
  assert(num == data.size());
}

void Context::BuildClientStubs() {
  assert(meta_.size() > 0);
  int srv_size = meta_.size();
  std::string postfix = "_client_stub.h";
  for (int srv_index = 0; srv_index < srv_size; ++srv_index) {
    std::string buf;
    buf += AddHeaderGuardStart(meta_[srv_index].first, "Client");
    buf += AddIncluder();
    buf += AddNameSpaceStart("leetrpc");
    buf += AddClassHeader(meta_[srv_index].first, "", "Client");
    buf += AddClientConstructor(meta_[srv_index].first, "Client");

    int func_size = meta_[srv_index].second.size();
    NodeFunc** fpp = &(meta_[srv_index].second[0]);
    for (int func_index = 0; func_index < func_size; ++func_index) {
      buf += AddClientFunc(fpp[func_index], srv_index, func_index);
      buf += "\r\n";
    }

    buf += AddClientData();
    buf += AddClassFooter();
    buf += AddNameSpaceEnd();
    buf += AddHeaderGuardEnd();
    std::string filename = meta_[srv_index].first;
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    filename += postfix;
    DumpToFile(filename, buf);
  }
}

std::string Context::AddServerMethodInterface(const std::vector<NodeFunc*>& funcs) {
  int size = static_cast<int>(funcs.size());
  std::string buf;
  for (int i = 0; i < size; ++i) {
    buf += AddServerFunc(funcs[i]); 
  }
  return buf;
}

void Context::BuildServerStubs() {
  assert(meta_.size() > 0);
  int srv_size = meta_.size();
  std::string postfix = "_server_stub.h";
  for (int srv_index = 0; srv_index < srv_size; ++srv_index) {
    std::string buf;
    buf += AddHeaderGuardStart(meta_[srv_index].first, "Server");
    buf += AddOneIncluder("leetrpc/dispatcher.h", 0);
    buf += AddIncluder();
    buf += AddNameSpaceStart("leetrpc");
    buf += AddClassHeader(meta_[srv_index].first, ": public Dispatcher", "Server");
    buf += AddServerConstructor(meta_[srv_index].first, "Server", srv_index);
    buf += AddServerMethodInterface(meta_[srv_index].second);
    buf += AddServerDispatcher(meta_[srv_index].second);
    buf += AddServerServiceId();
    buf += AddServerData();
    buf += AddClassFooter();
    buf += AddNameSpaceEnd();
    buf += AddHeaderGuardEnd();
    std::string filename = meta_[srv_index].first;
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    filename += postfix;
    DumpToFile(filename, buf);
  }
}

void Context::BuildMeta(AST *a) {
  assert(a && a->type == kCLASS);
  if (!a->l) PrintError("Error: No service name!");
  std::string cls = (reinterpret_cast<NodeName*>(a->l))->s;
  if (!a->r) PrintError("Error: No service method defined!");
  meta_.push_back(std::make_pair(cls, std::vector<NodeFunc*>()));
  NodeBody* body = reinterpret_cast<NodeBody*>(a->r);
  NodeFunc* head = body->func;
  while (head) {
    meta_.back().second.push_back(head);
    head = head->next;
  }
}

void Context::Start(AST* a) {
  BuildMeta(a);
  BuildClientStubs();
  BuildServerStubs();
}

} // namespace yy
