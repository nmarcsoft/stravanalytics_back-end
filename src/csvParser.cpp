#include "csvParser.hpp"

Parser::Parser(std::string input) {
    logger.log(INFO, "Parser Instanciate.");
    this->input = std::ifstream(input);
}

Parser::~Parser()
{
    logger.log(INFO, "Parser Killed.");
}

string Parser::parse(string argument)
{
    string str = "";
    while ( std::getline(this->input, str) )
    {
        size_t pos = str.find(';');
        if (pos != std::string::npos)
        {
            std::string key = str.substr(0, pos);
            std::string value = str.substr(pos + 1);
            
            if (key == argument)
            {
                return value;
            }
        }
        else
        {
            logger.log(WARNING, str);
        }
    }
    return 0;
}