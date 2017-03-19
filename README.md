# LeetRPC

Yet another lightweight rpc library for C\+\+.

# Features

- Using RPC calls like calling regular local function
- Support asynchronous call and responding error callback
- Support timeout mechanism for synchronous and asynchronous call
- Support C++ types: `bool`, `double`, `string`, `vector<T>`, `map<T>`
- Using code-generator to define service easily without knowing any serialization involved

# Required

- Linux(2.6+)
- GCC(４.8+)
- flex(2.5.39+)
- bison(3.0.2+)

# Demo

__Compile the code generator__
```shell
./codegen.sh
```
Now we have the code generator `conv`.

__Scratch your service declaration__
```IDL
service Add {
  double Add(double a, double b);
}
```
Save it as input.co and parse it using code generator 'conv'

```shell
./conv < input.co
```
Two files will be generated: `add_client_stub.h` and `add_server_stub.h`. Next step you should take is to create a new file, e.g. `add_server_stub_def.h`, inherit the class defined in `add_server_stub.h` and complete the definition of method `Add`.

```C++
#ifndef LEETRPC_ADD_SERVER_STUB_DEF_H__
#define LEETRPC_ADD_SERVER_STUB_DEF_H__

#include "add_server_stub.h"

namespace leetrpc {
class AddServerStubDef: public AddServerStub {
 public:
  double Add(double a, double b) {
    return a + b;
  }
};
} // namespace leetrpc
#endif // LEETRPC_ADD_SERVER_STUB_DEF_H__
```
__Client use case__
```C++
  ...
  RpcClient client("127.0.0.1", 8080);
  AddClientStub stub(client);
  int ret = stub.Add(1, 2);
  ...

```

__Server use case__
```C++
  ... 
  RpcServer srv("127.0.0.1", 8080, 4);
  srv.Register(new AddServerStubDef());
  srv.Run();
```


