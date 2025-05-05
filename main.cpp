#include "json/json.h"
#include <iostream>

int main() {
  std::string source = 
  R"({"k1": 123, "k2": false, "k3": 1.23, "k4": "va\"lue4\"", "k5": [1, 2, 3], "k6": {"sk1": 456, "sk2": 4.56}})";

  Json json = Json::parse(source);
  int k1 = json["k1"];
  std::cout << k1 << std::endl;
  JsonArray k5 = json["k5"];
  int n5 = k5[1];
  std::cout << n5 << std::endl;
  
}
