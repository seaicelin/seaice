#ifndef __SEAICE_SERVLET_H__
#define __SEAICE_SERVLET_H__

#include "http.h"
#include "http_session.h"
#include "../mutex.h"

#include <functional>
#include <memory>
#include <stdint.h>
#include <unordered_map>

namespace seaice {
namespace http {

class Servlet {
public:
    typedef std::shared_ptr<Servlet> ptr;
    Servlet(const std::string& name) 
        : m_name(name) {
    }

    virtual ~Servlet() {}
    virtual int32_t handle(seaice::http::HttpRequest::ptr request
                    , seaice::http::HttpResponse::ptr response
                    , seaice::http::HttpSession::ptr session) = 0;
    const std::string& getName() const {return m_name;}

protected:
    std::string m_name;
};

class FunctionServlet : public Servlet {
public:
    typedef std::shared_ptr<FunctionServlet> ptr;
    typedef std::function<int32_t (seaice::http::HttpRequest::ptr request
                    , seaice::http::HttpResponse::ptr response
                    , seaice::http::HttpSession::ptr session)> Callback;
    FunctionServlet(Callback cb);
    virtual int32_t handle(seaice::http::HttpRequest::ptr request
                    , seaice::http::HttpResponse::ptr response
                    , seaice::http::HttpSession::ptr session) override;

private:
    Callback m_cb;
};

class ServletDispatch : public Servlet {
public:
    typedef std::shared_ptr<ServletDispatch> ptr;
    typedef RWMutex RWMutexType;

    ServletDispatch();
    virtual int32_t handle(seaice::http::HttpRequest::ptr request
                    , seaice::http::HttpResponse::ptr response
                    , seaice::http::HttpSession::ptr session) override;

    void addServlet(const std::string& uri, Servlet::ptr slt);
    void addServlet(const std::string& uri, FunctionServlet::Callback cb);
    void addGlobServlet(const std::string& uri, Servlet::ptr slt);
    void addGlobServlet(const std::string& uri, FunctionServlet::Callback cb);

    void delServlet(const std::string& name);
    void delGlobServlet(const std::string& name);

    Servlet::ptr getDefault() const {return m_default;};
    void setDefault(Servlet::ptr v) {m_default = v;};
    Servlet::ptr getServlet(const std::string& uri);
    Servlet::ptr getGlobServlet(const std::string& uri);
    Servlet::ptr getMatchServlet(const std::string& uri);

private:
    Servlet::ptr m_default;
    RWMutexType m_mutex;
    std::unordered_map<std::string, Servlet::ptr> m_datas;
    std::vector<std::pair<std::string, Servlet::ptr> > m_globs;
};

class NotFoundServlet : public Servlet {
public:
    typedef std::shared_ptr<NotFoundServlet> ptr;
    NotFoundServlet(const std::string& name);
    virtual int32_t handle(seaice::http::HttpRequest::ptr request
                    , seaice::http::HttpResponse::ptr response
                    , seaice::http::HttpSession::ptr session) override;
private:
    std::string m_name;
    std::string m_content;
};

}
}

#endif