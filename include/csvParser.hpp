#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <bits/stdc++.h>

#include "Logger.hpp"

class Parser {
private:
    std::ifstream input;
    std::vector<std::string> result;

public :
    Parser(std::string path);
    std::string parse(std::string arg);
    ~Parser();
};