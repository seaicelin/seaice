#ifndef __SEAICE_CONFIG2_H__
#define __SEAICE_CONFIG2_H__

#include <memory>
#include <functional>
#include <stdint.h>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <unordered_set>
#include <unordered_map>
#include "thread.h"
#include "log.h"
#include "yaml-cpp/yaml.h"
#include "utils.h"

namespace seaice {

class ConfigVarBase
{
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& descriptrion = "") 
        : m_name(name) 
        , m_desc(descriptrion) {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }
    virtual ~ConfigVarBase() {};

    const std::string& getName() const {return m_name;}
    const std::string& getDescription() const {return m_desc;}

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& v) = 0;
    virtual std::string getTypeName() const = 0;
protected:
    std::string m_name;
    std::string m_desc;
};

template<typename F, typename T>
class LexicalCast
{
public:
    T operator()(const F& f) {
        return boost::lexical_cast<T>(f);
    }
};


template<typename T>
class LexicalCast<std::string, std::vector<T> > {
public:
    std::vector<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); i++) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<typename T>
class LexicalCast<std::vector<T>, std::string>
{
public:
    std::string operator()(const std::vector<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<typename T>
class LexicalCast<std::string, std::list<T> >
{
public:
    std::list<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::list<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); i++) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<typename T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator() (std::list<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<typename T>
class LexicalCast<std::string, std::set<T> >
{
public:
    std::set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); i++) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<typename T>
class LexicalCast<std::set<T>, std::string>
{
public:
    std::string operator()(std::set<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<typename T>
class LexicalCast<std::string, std::unordered_set<T> >
{
public:
    std::unordered_set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); i++) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<typename T>
class LexicalCast<std::unordered_set<T>, std::string>
{
public:
    std::string operator()(const std::unordered_set<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


template<typename T>
class LexicalCast<std::string, std::map<std::string, T> >
{
public:
    std::map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> vec;
        stringstream ss;
        for(auto it = node.begin();
                it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

template<typename T>
class LexicalCast<std::map<std::string, T>, std::string>
{
public:
    std::string operator()(std::map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<typename T>
class LexicalCast<std::string, std::unordered_map<std::string, T> >
{
public:
    std::unordered_map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin();
                it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(),
                        LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

template<typename T>
class LexicalCast<std::unordered_map<std::string, T>, std::string>
{
public:
    std::string operator()(const std::unordered_map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<typename T, typename FromStr = LexicalCast<std::string, T>
                    ,typename ToStr = LexicalCast<T, std::string> >
class ConfigVar : public ConfigVarBase {
public:
    typedef RWMutex RWMutexType;
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;

    ConfigVar(const std::string& name
            , const T& default_value
            , const std::string descriptrion = "")
        : ConfigVarBase(name, descriptrion)
        , m_val(default_value) {
    }

    ~ConfigVar() {}

    std::string toString() override {
        try {
            RWMutexType::ReadLock lock(m_mutex);
            return ToStr()(m_val);
        } catch (std::exception& e) {
            SEAICE_LOG_ERROR(SEAICE_LOGGER("system")) << "ConfigVar::toString exception "
                << e.what() << " convert: " << seaice::utils::TypeToName<T>() << " to string"
                <<" name = " << m_name;
        }
        return "";
    }

    bool fromString(const std::string& val) override {
        try{
            setValue(FromStr()(val));
        } catch (std::exception& e) {
            SEAICE_LOG_ERROR(SEAICE_LOGGER("system")) << "ConfigVar::fromString exception "
                << e.what() << " convert string to " << seaice::utils::TypeToName<T>()
                <<" name = " << m_name
                << " - " << val;
        }
        return false;
    }

    const T getValue() {
        RWMutexType::ReadLock lock(m_mutex);
        return m_val;
    }

    void setValue(const T& t) {
        {
            RWMutexType::ReadLock lock(m_mutex);
            if(m_val == t) {
                return;
            }
            for(auto& i : m_cbs) {
                i.second(m_val, t);
            }
        }
        RWMutexType::WriteLock lock(m_mutex);
        m_val = t;
    }

    std::string getTypeName() const override {return seaice::utils::TypeToName<T>();}

    uint64_t addListener(on_change_cb cb) {
        static uint64_t s_cb_id = 0;
        RWMutexType::WriteLock lock(m_mutex);
        ++s_cb_id;
        m_cbs[s_cb_id] = cb;
        return s_cb_id;
    }

    void delListener(uint64_t key) {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.erase(key);
    }

    on_change_cb getListener(uint64_t key) {
        RWMutexType::ReadLock lock(m_mutex);
        auto it = m_cbs.find(key);
        return it == m_cbs.end()? nullptr : it->second;
    }

    void clearListener() {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.clear();
    }

private:
    T m_val;
    RWMutexType m_mutex;
    std::map<uint64_t, on_change_cb> m_cbs;
};



class Config2
{
public:
    typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    typedef RWMutex RWMutexType;

    template<typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name
                                            ,const T& default_value
                                            , const std::string& descriptrion = "") {
        RWMutexType::WriteLock lock(GetMutex());
        auto it = GetDatas().find(name);
        if(it != GetDatas().end()) {
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
            if(tmp) {
                SEAICE_LOG_INFO(SEAICE_LOGGER("system")) << "Lookup name = " << name << " exist";
                return tmp;
            } else {
                SEAICE_LOG_ERROR(SEAICE_LOGGER("system")) <<"Lookup name = " << name << " exist but type not "
                    << seaice::utils::TypeToName<T>() << " real_type = " << it->second->getTypeName()
                    << " " << it->second->toString();
                return nullptr;
            }
        }
        if(name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678")
            != std::string::npos) {
            throw std::invalid_argument(name);
        }
        //typename 声明类型
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, descriptrion));
        GetDatas()[name] = v;
        return v;
    }

    template<typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        RWMutexType::WriteLock lock(GetMutex());
        auto it = GetDatas().find(name);
        if(it == GetDatas().end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    }

    static void LoadFromYaml(const YAML::Node& root);
    static void LoadFromConfDir(const std::string& path, bool force = false);
    static ConfigVarBase::ptr LookupBase(const std::string& name);
    static void Visit(std::function<void (ConfigVarBase::ptr)> cb);
private:
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }
    static RWMutexType& GetMutex() {
        static RWMutexType s_mutex;
        return s_mutex;
    }
};

}

#endif