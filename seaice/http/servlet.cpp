#include "servlet.h"
#include <fnmatch.h>

namespace seaice {
namespace http {

FunctionServlet::FunctionServlet(Callback cb) 
    : Servlet("FunctionServlet")
    , m_cb(cb) {
}

int32_t FunctionServlet::handle(seaice::http::HttpRequest::ptr request
                    , seaice::http::HttpResponse::ptr response
                    , seaice::http::HttpSession::ptr session) {
    m_cb(request, response, session);
}

ServletDispatch::ServletDispatch() 
    : Servlet("ServletDispatch") {
    m_default.reset(new NotFoundServlet("seaice/1.0"));
}

int32_t ServletDispatch::handle(seaice::http::HttpRequest::ptr request
                , seaice::http::HttpResponse::ptr response
                , seaice::http::HttpSession::ptr session) {
    auto slt = getMatchServlet(request->getPath());
    if(slt) {
        slt->handle(request, response, session);
    }
    return 0;
}

void ServletDispatch::addServlet(const std::string& uri, Servlet::ptr slt) {
    RWMutexType::WriteLock lock(m_mutex);
    m_datas[uri] = slt;
}

void ServletDispatch::addServlet(const std::string& uri, FunctionServlet::Callback cb) {
    RWMutexType::WriteLock lock(m_mutex);
    FunctionServlet::ptr slt(new FunctionServlet(cb));
    m_datas[uri] = slt;
}

void ServletDispatch::addGlobServlet(const std::string& uri, Servlet::ptr slt) {
    RWMutexType::WriteLock lock(m_mutex);
    for(auto it = m_globs.begin();
            it != m_globs.end(); it++) {
        if(it->first == uri) {
            m_datas.erase(uri);
            break;
        }
    }
    m_globs.push_back(std::make_pair(uri, slt));
}

void ServletDispatch::addGlobServlet(const std::string& uri, FunctionServlet::Callback cb) {
    return addGlobServlet(uri, std::make_shared<FunctionServlet>(cb));
}

void ServletDispatch::delServlet(const std::string& name) {
    RWMutexType::WriteLock lock(m_mutex);
    m_datas.erase(name);
}

void ServletDispatch::delGlobServlet(const std::string& name) {
    RWMutexType::WriteLock lock(m_mutex);
    for(auto it = m_globs.begin();
            it != m_globs.end(); it++) {
        if(it->first == name) {
            m_globs.erase(it);
            break;
        }
    }
}

Servlet::ptr ServletDispatch::getServlet(const std::string& uri) {
    RWMutexType::WriteLock lock(m_mutex);
    auto it = m_datas.find(uri);
    return it == m_datas.end() ? nullptr : it->second;
}

Servlet::ptr ServletDispatch::getGlobServlet(const std::string& uri) {
    RWMutexType::WriteLock lock(m_mutex);
    for(auto it = m_globs.begin(); 
            it != m_globs.end(); it++) {
        if(it->first == uri) {
            return it->second;
        }
    }
    return nullptr;
}

Servlet::ptr ServletDispatch::getMatchServlet(const std::string& uri) {
    RWMutexType::WriteLock lock(m_mutex);
    auto mit = m_datas.find(uri);
    if(mit != m_datas.end()) {
        return mit->second;
    }
    for(auto it = m_globs.begin();
            it != m_globs.end(); it++) {
        if(!fnmatch(it->first.c_str(), uri.c_str(), 0)) {
            return it->second;
        }
    }
    return m_default;
} 

NotFoundServlet::NotFoundServlet(const std::string& name) 
    : Servlet("NotFoundServlet")
    , m_name(name) {
    m_content = "<html><head><title>404 Not Found"
        "</title></head><body><center><h1>404 Not Found</h1></center>"
        "<hr><center>" + name + "</center></body></html>";
}

int32_t NotFoundServlet::handle(seaice::http::HttpRequest::ptr request
                , seaice::http::HttpResponse::ptr response
                , seaice::http::HttpSession::ptr session) {
    response->setStatus(seaice::http::HttpStatus::NOT_FOUND);
    response->getHeader("Server", "seaice/1.0.0");
    response->setHeader("Content-Type", "text/html");
    response->setBody(m_content);
}



}
}