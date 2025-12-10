#pragma once

#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Logger.hpp"

class Parser {
private:
  std::ifstream input;
  std::vector<std::string> result;

public:
  Parser(std::string path);
  std::string parse(std::string arg);
  ~Parser();
};
