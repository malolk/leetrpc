#ifndef LEETRPC_CONTEXT_H__
#define LEETRPC_CONTEXT_H__

#include "ast.h"

#include <string>
#include <vector>
#include <utility>
#include <iostream>

namespace yy {

class Context {
 public:
  void Start(AST* a);
  // void Free(AST* a);
 private:
  std::string AddArg(NodeArg* a, const std::string& refs);
  std::string AddClassFooter();
  std::string AddClassHeader(const std::string& cls, const std::string& extends, const std::string& name);
  std::string AddClientConstructor(const std::string& cls, const std::string& port);
  std::string AddClientData();
  std::string AddClientFunc(NodeFunc* f, int s_id, int f_id, std::string& pri_buf);
  std::string AddFuncSignature(NodeFunc* f);
  std::string AddHeaderGuardEnd();
  std::string AddHeaderGuardStart(std::string name, std::string port);
  std::string AddIncluder();
  std::string AddIndentation(int s);
  std::string AddNameSpaceEnd();
  std::string AddNameSpaceStart(const std::string& name);
  std::string AddOneIncluder(const std::string& header, int is_std);
  std::string AddServerConstructor(const std::string& cls, const std::string& port, int service_id);
  std::string AddServerData();
  std::string AddServerDispatcher(const std::vector<NodeFunc*>& funcs);
  std::string AddServerFunc(NodeFunc* f);
  std::string AddServerFunc(NodeFunc* f, int s_id, int f_id);
  std::string AddServerMethodDispatcher(const std::vector<NodeFunc*>& funcs);
  std::string AddServerMethodInterface(const std::vector<NodeFunc*>& funcs);
  std::string AddServerOneMethodDispatcher(const NodeFunc* f);
  std::string AddServerServiceId();
  std::string GenerateClassName(const std::string& cls, const std::string& name);
  void BuildClientStubs();
  void BuildMeta(AST *a);
  void BuildServerStubs();

  typedef std::vector<NodeFunc*> FuncArray;
  typedef std::pair<std::string, FuncArray> ServiceType;
  typedef std::vector<ServiceType> MetaType;
  MetaType meta_;
};

} // namespace yy
#endif // LEETRPC_CONTEXT_H__

