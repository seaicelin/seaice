#include "config2.h"
#include "env.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace seaice {

static Logger::ptr logger = SEAICE_LOGGER("system");

ConfigVarBase::ptr Config2::LookupBase(const std::string& name) {
    RWMutexType::ReadLock lock(GetMutex());
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
}

static void ListAllMember(const std::string& prefix
                        , const YAML::Node& node
                        , std::list<std::pair<std::string, const YAML::Node> >& output) {
    if(prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678")
        != std::string::npos) {
        SEAICE_LOG_ERROR(logger) << "Config invalid name " << prefix <<" : "<<node;
        return;
    }
    output.push_back(std::make_pair(prefix, node));
    if(node.IsMap()) {
        for(auto it = node.begin();
                it != node.end(); ++it) {
            ListAllMember(prefix.empty()? it->first.Scalar()
                    : prefix + "." + it->first.Scalar(), it->second, output);
        }
    }
}

void Config2::LoadFromYaml(const YAML::Node& root) {
    std::list<std::pair<std::string, const YAML::Node> > all_nodes;
    ListAllMember("", root, all_nodes);

    for(auto& i : all_nodes) {
        std::string key = i.first;
        if(key.empty()) {
            continue;
        }
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        ConfigVarBase::ptr var = LookupBase(key);
        if(var) {
            if(i.second.IsScalar()) {
                var->fromString(i.second.Scalar());
            } else {
                std::stringstream ss;
                ss << i.second;
                var->fromString(ss.str());
            }
        }
    }
}

static std::map<std::string, uint64_t> s_file2Modifytime;
static seaice::Mutex s_mutex;

void Config2::LoadFromConfDir(const std::string& path, bool force) {
    std::string absoulte_path = seaice::EnvMgr::GetInstance()->getAbsolutePath(path);
    std::vector<std::string> files;
    seaice::utils::FSUtil::ListAllFile(files, absoulte_path, ".yml");

    for(auto& i : files) {
        {
            struct stat st;
            lstat(i.c_str(), &st);
            seaice::Mutex::Lock lock(s_mutex);
            if(!force && s_file2Modifytime[i] == (uint64_t)st.st_mtime) {
                continue;
            }
            s_file2Modifytime[i] = st.st_mtime;
        }
        try {
            YAML::Node root = YAML::LoadFile(i);
            LoadFromYaml(root);
            SEAICE_LOG_DEBUG(logger) << "LoadConfFile file = " << i << " OK";
        } catch (...) {
            SEAICE_LOG_ERROR(logger) << "LoadConfFile file = " << i << " failed";
        }
    }
}

void Config2::Visit(std::function<void (ConfigVarBase::ptr)> cb) {
    RWMutexType::ReadLock lock(GetMutex());
    ConfigVarMap& m = GetDatas();
    for(auto it = m.begin();
            it != m.end(); ++it) {
        cb(it->second);
    }
}

}