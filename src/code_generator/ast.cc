#include "ast.h"

#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <string>

namespace yy {

void PrintError(const std::string& msg) {
  std::cerr << msg << std::endl;
  assert(0);
}

char* AllocWrapper(int size, const char* func) {
  assert(size > 0);
  char* p = reinterpret_cast<char*>(malloc(size));
  if (!p) PrintError(std::string(func) + ": " + "out of space.");
  return p;
}

AST* NewClass(const char* cls, AST* body) {
  AST* node = reinterpret_cast<AST*>(AllocWrapper(sizeof(AST), "NewClass"));
  NodeName* val = reinterpret_cast<NodeName*>(AllocWrapper(sizeof(NodeName), "NewClass"));
  val->type = kNAME;
  val->s = cls;
  node->type = kCLASS;
  node->l = reinterpret_cast<AST*>(val);
  node->r = body;
  return node;
}

AST* NewBody(AST* body, AST* func) {
  if (!body) {
    NodeBody* node = reinterpret_cast<NodeBody*>(AllocWrapper(sizeof(NodeBody), "NewBody"));
    node->type = kBODY;
    node->func = reinterpret_cast<NodeFunc*>(func);
    return reinterpret_cast<AST*>(node);
  } else {
    NodeFunc* head = (reinterpret_cast<NodeBody*>(body))->func;
    while (head && head->next) head = head->next;
    head->next = reinterpret_cast<NodeFunc*>(func);
    return body;
  }
}

AST* NewFunc(AST* arg, const char* type, const char* name) {
  NodeFunc* node = reinterpret_cast<NodeFunc*>(AllocWrapper(sizeof(NodeFunc), "NewFunc"));
  node->type = kFUNC;
  node->ret_t = type;
  node->name = name;
  node->arg = arg;
  node->next = NULL;
  return reinterpret_cast<AST*>(node);
}

AST* NewArg(AST* arg, const char* type, const char* name) {
  NodeArg* node = reinterpret_cast<NodeArg*>(AllocWrapper(sizeof(NodeArg), "NewArg"));
  node->type = kARG;
  node->arg_t = type;
  node->name = name;
  node->next = NULL;
  if (arg == NULL) return reinterpret_cast<AST*>(node);
  NodeArg* head = reinterpret_cast<NodeArg*>(arg);
  assert(head->type == kARG);
  while (head && head->next) head = head->next;
  head->next = node;
  head = reinterpret_cast<NodeArg*>(arg);
  while (head) {
    head = head->next;
  }
  return arg;
}

} // namespace yy
