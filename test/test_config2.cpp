#include "test.h"
#include <time.h>
#include <sys/stat.h>
#include "../seaice/daemon.h"
#include "../seaice/env.h"
#include <fstream>
#include "../seaice/config2.h"

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

seaice::ConfigVar<int>::ptr g_int_value_config = 
    seaice::Config2::Lookup("system.port", (int)8080, "system port");

//seaice::ConfigVar<float>::ptr g_int_value_config2 = 
//    seaice::Config2::Lookup("system.port", (float)8080, "system port");

seaice::ConfigVar<float>::ptr g_float_value_config = 
    seaice::Config2::Lookup("system.value", (float)1.2f, "system value");

seaice::ConfigVar<std::vector<int> >::ptr g_int_vec_config = 
    seaice::Config2::Lookup("system.int_vec", std::vector<int>{1, 2}, "system int vec");

seaice::ConfigVar<std::list<int> >::ptr g_int_list_config = 
    seaice::Config2::Lookup("system.int_list", std::list<int>{1, 2}, "system int list");

seaice::ConfigVar<std::set<int> >::ptr g_int_set_config = 
    seaice::Config2::Lookup("system.int_set", std::set<int>{1, 2}, "system int set");

seaice::ConfigVar<std::unordered_set<int> >::ptr g_int_unordered_set_config = 
    seaice::Config2::Lookup("system.int_unordered_set", std::unordered_set<int>{1, 2, 3}, "system int unordered_set");

seaice::ConfigVar<std::map<std::string, int> >::ptr g_int_map_config = 
    seaice::Config2::Lookup("system.int_map", std::map<std::string, int>{{"k", 2}}, "system int map");

seaice::ConfigVar<std::unordered_map<std::string, int> >::ptr g_int_unordered_map = 
    seaice::Config2::Lookup("system.int_ordered_map", std::unordered_map<std::string, int>{{"k", 2}}, "system ordered map");

void test_config() {
    SEAICE_LOG_DEBUG(logger) << "before: " << g_int_value_config->getValue();
    //SEAICE_LOG_DEBUG(logger) << "before: " << g_int_value_config2->getValue();
    SEAICE_LOG_DEBUG(logger) << "before: " << g_float_value_config->getValue();

#define XX(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for(auto& i : v) { \
            SEAICE_LOG_DEBUG(logger) << #prefix " " #name ": " << i; \
        }\
        SEAICE_LOG_DEBUG(logger) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

#define XXM(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for(auto& i : v) { \
            SEAICE_LOG_DEBUG(logger) << #prefix " " #name ": {" << i.first \
                << ", " << i.second <<"}"; \
        } \
        SEAICE_LOG_DEBUG(logger) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

    XX(g_int_vec_config, int_vec, before);
    XX(g_int_list_config, int_list, before);
    XX(g_int_set_config, int_set, before);
    XX(g_int_unordered_set_config, int_unordered_set, before);
    XXM(g_int_map_config, int_map, before);
    XXM(g_int_unordered_map, int_unordered_map, before);

    YAML::Node root = YAML::LoadFile("/media/sf_Iceserver/vsclient/seaice/build/bin/conf/test.yml");
    seaice::Config2::LoadFromYaml(root);

    SEAICE_LOG_DEBUG(logger) << "after: " << root;

    SEAICE_LOG_DEBUG(logger) << "after: " << g_int_value_config->getValue();
    SEAICE_LOG_DEBUG(logger) << "after: " << g_float_value_config->getValue();
    XX(g_int_vec_config, int_vec, after);
    XX(g_int_list_config, int_list, after);
    XX(g_int_set_config, int_set, after);
    XX(g_int_unordered_set_config, int_unordered_set, after);
    XXM(g_int_map_config, int_map, after);
    XXM(g_int_unordered_map, int_unordered_map, after);
}

void test_yaml() {
    YAML::Node root = YAML::LoadFile("/media/sf_Iceserver/vsclient/seaice/build/bin/conf/test.yml");
    SEAICE_LOG_DEBUG(logger) << root["languages"].IsDefined();
    SEAICE_LOG_DEBUG(logger) << root["languages2"].IsDefined();
    SEAICE_LOG_DEBUG(logger) << root;
    SEAICE_LOG_DEBUG(logger) << root["languages2"];
}

void testLoadYaml() {
    seaice::Config2::LoadFromConfDir("conf");
}

class Person
{
public:
    Person() 
        : m_name("unknow")
        , m_age(19)
        , m_sex(true){
    }
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name = " << m_name
           << " age = " << m_age
           << " sex = " << m_sex
           << "]";
        return ss.str();
    }
    bool operator == (const Person& rhs) const {
        return m_name == rhs.m_name
            && m_age == rhs.m_age
            && m_sex == rhs.m_sex;
    }
};

namespace seaice {

template<>
class LexicalCast<std::string, Person> {
public:
    Person operator() (const std::string& v) {
        //std::cout<< "v = " << v << std::endl;
        YAML::Node node = YAML::Load(v);
        //std::cout<< "node = " << node << std::endl;
        Person p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        //std::cout<< p.toString() << std::endl;
        return p;
    }
};

template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person& p) {
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        stringstream ss;
        ss << node;
        return ss.str();
    }
};

}

seaice::ConfigVar<Person>::ptr g_person = 
    seaice::Config2::Lookup("class.person", Person(), "system person");

seaice::ConfigVar<std::map<std::string, Person> >::ptr g_person_map = 
    seaice::Config2::Lookup("class.map", std::map<std::string, Person>(), "system person");

seaice::ConfigVar<std::map<std::string, std::vector<Person> > >::ptr g_person_vecmap = 
    seaice::Config2::Lookup("class.vec_map", std::map<std::string, std::vector<Person> >(), "system person");

void test_class() {
    SEAICE_LOG_DEBUG(logger) << "before: " << g_person->getValue().toString()
        << " - " << g_person->toString();
#define XX_PM(g_var, prefix) \
        {\
            auto m = g_var->getValue(); \
            for(auto& i : m) { \
                SEAICE_LOG_DEBUG(logger) << prefix " ： " << i.first << " - "  \
                    << i.second.toString(); \
            } \
            SEAICE_LOG_DEBUG(logger) << prefix << ": size = " << m.size(); \
        }

    g_person->addListener([](const Person& old_value, const Person& new_value) {
        SEAICE_LOG_DEBUG(logger) << "old_value = " << old_value.toString()
            << " new_value = " << new_value.toString();
    });

    XX_PM(g_person_map, "class.map before");
    SEAICE_LOG_DEBUG(logger) << "before: " << g_person_vecmap->toString();

    YAML::Node root = YAML::LoadFile("/media/sf_Iceserver/vsclient/seaice/build/bin/conf/test.yml");
    seaice::Config2::LoadFromYaml(root);

    SEAICE_LOG_DEBUG(logger) << "after: " << g_person->getValue().toString()
        << " - " << g_person->toString();

    XX_PM(g_person_map, "class.map after");
    SEAICE_LOG_DEBUG(logger) << "after: " << g_person_vecmap->toString();

}

void test_log() {
    static seaice::Logger::ptr logger = SEAICE_LOGGER("system");
    SEAICE_LOG_DEBUG(logger) << " hello log ";
    std::cout << seaice::LoggerMgr::getInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("/media/sf_Iceserver/vsclient/seaice/build/bin/conf/log.yml");
    seaice::Config2::LoadFromYaml(root);
    //std::cout << "=================" << std::endl;
    //std::cout << seaice::LoggerMgr::getInstance()->toYamlString() << std::endl;
    //std::cout << "=================" << std::endl;
    //std::cout << root << std::endl;
    logger = SEAICE_LOGGER("system");
    //logger->setFormatter("%d - %m%n");
    SEAICE_LOG_DEBUG(logger) << " hello log2 ";
    logger = SEAICE_LOGGER("system");
    //logger->setFormatter("%d - %m%n");
    SEAICE_LOG_DEBUG(logger) << " hello log2 ";
    logger = SEAICE_LOGGER("root");
    SEAICE_LOG_DEBUG(logger) << " hello log3 ";
}

int main(int argc, char *argv[])
{
    seaice::EnvMgr::GetInstance()->init(argc, argv);
    testLoadYaml();
    sleep(5);
    testLoadYaml();
    test_yaml();
    test_config();
    test_class();
    test_log();
    return 0;
}