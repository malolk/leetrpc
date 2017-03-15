#ifndef LEETRPC_ADD_SERVER_STUB_DEF_H__
#define LEETRPC_ADD_SERVER_STUB_DEF_H__

#include "add_server_stub.h"
#include "libbase/loggerutil.h"

namespace leetrpc {
class AddServerStubDef: public AddServerStub {
 public:
  double Add(double a, double b) {
    LOG_INFO("Params: a = %f, b = %f", a, b);
    return a + b;
  } 
};
} // namespace leetrpc
#endif // LEETRPC_ADD_SERVER_STUB_DEF_H__
