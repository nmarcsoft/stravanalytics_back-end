#include "Logger.hpp"
#include "csvParser.hpp"
#include "Connector.hpp"

int main(void)
{
    logger.log(INFO, "Program Started.");

    // Récupération de l'acess_token
    Parser test = Parser("../var.csv");
    std::string access_token = test.parse("YOURACCESSTOKEN");
    
    // Connection à strava
    ConnectionManager connection_manager (access_token);
    connection_manager.test_connection();
    
    return 0;
}