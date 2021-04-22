#ifndef __SEAICE_APPLICATION_H__
#define __SEAICE_APPLICATION_H__

namespace seaice{

class Application {
public:
    Application();

    static Application* GetInstance() {return s_application;}
    bool init(int argc, char** argv);
    void run();
private:
    int main(int argc, char** argv);
private:
    int m_argc = 0;
    char** argv = nullptr;
    static Application* s_instance;
};

}
#endif