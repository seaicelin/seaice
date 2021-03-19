#include <iostream>
#include "yaml-cpp/yaml.h"
#include "../seaice/log.h"

using namespace std;

int main(int argc, char** args)
{
    YAML::Node config = YAML::LoadFile("../seaice/config/logConfig.yaml");
    /*
    cout<< "name = " <<config["name"].as<string>() << endl;
    for(YAML::const_iterator it = config["skills"].begin(); 
            it != config["skills"].end(); it++) {
        cout << it->first.as<string>() << " : " << it->second.as<int>() << endl;
    }
    */

    YAML::Node loggers = config["loggers"];
    cout << "loggers size = " << loggers.size() <<endl;

    for(auto i = 0; i < loggers.size(); i++)
    {
        YAML::Node logger = loggers[i];
        cout << "id = " << logger["id"].as<string>() << endl;
        cout << "level = " << logger["level"].as<string>() << endl;
        cout << "name = " << logger["name"].as<string>() << endl;
        YAML::Node appenders = logger["appenders"];
        for(auto j = 0; j < appenders.size(); j++)
        {
            cout << "appenders size = " << appenders.size() <<endl;
            YAML::Node appender = appenders[j];
            cout << "type = " << appender["type"].as<int>() << endl;
            cout << "formatter pattern = " << appender["formatter"]["pattern"].as<string>() << endl;
        }
        cout << "formatter pattern = " << logger["formatter"]["pattern"].as<string>() << endl;
    }

    return 0;
}