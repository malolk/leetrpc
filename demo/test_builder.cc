#include "jsonutil/json.h"
#include <iostream>
#include <string>

using namespace jsonutil;
using namespace std;
int main() {
  Value data(kJSON_ARRAY);
  Builder<Value> batch;
  batch << 1 << std::string("12");
  batch << 2;
  data.MergeArrayBuilder(batch);
  std::cout << data.ToString() << std::endl;  
}
