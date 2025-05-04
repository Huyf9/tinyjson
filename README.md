# TinyJson

&emsp;&emsp;一个玩具级别的Json解析工具。使用C++17实现。

[![CMake on multiple platforms](https://github.com/Huyf9/tinyjson/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/Huyf9/tinyjson/actions/workflows/cmake-multi-platform.yml)
[![language](https://img.shields.io/badge/Language-C++17-green)]()
[![author](https://img.shields.io/badge/Author-hyf-blue)]()

## 如何使用

```cpp
#include "json/json.h"
#include <string>
#include <iostream>


int main() {
  std::string source = 
  "{\"k1\": 123, \"k2\": false, \"k3\": 1.23, \"k4\": \"value4\", \"k5\": [1, 2, 3], \"k6\": {\"sk1\": 456, \"sk2\": 4.56}}";

  Json json = Json::parse(source);
  int k1 = json["k1"];
  std::cout << k1 << std::endl;
  JsonArray k5 = json["k5"];
  int n5 = k5[1];
  std::cout << n5 << std::endl;
}

```

## 后续计划

1. 序列化与反序列化
2. 异常优化

【备注】 都不想做

