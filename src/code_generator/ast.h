#ifndef RPC_AST_H__
#define RPC_AST_H__

#include <string>

namespace yy {
typedef enum {
  kCLASS,
  kBODY,
  kFUNC,
  kARG,
  kNAME
} NodeType;

struct AST;
typedef struct AST AST;
struct AST {
  NodeType type;
  AST* l;
  AST* r;
};

typedef struct {
  NodeType type;
  const char* s;
} NodeName;

struct NodeFunc;
typedef struct NodeFunc NodeFunc;
struct NodeFunc {
  NodeType type;
  const char* ret_t;
  const char* name;
  AST* arg;
  NodeFunc* next;
};

typedef struct {
  NodeType type;
  NodeFunc* func;
} NodeBody;


struct NodeArg;
typedef struct NodeArg NodeArg;
struct NodeArg {
  NodeType type;
  const char* arg_t;
  const char* name;
  NodeArg* next;
};

AST* NewClass(const char* cls_name, AST* body);
AST* NewBody(AST* body, AST* func);
AST* NewFunc(AST* arg, const char* type, const char* name);
AST* NewArg(AST* arg_list, const char* type, const char* name);
void PrintError(const std::string& msg);

} // namespace yy
#endif // RPC_AST_H__
