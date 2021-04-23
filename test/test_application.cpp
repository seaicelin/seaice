#include "test.h"
#include "../seaice/application.h"

int main(int argc, char *argv[])
{
    seaice::Application app;
    if(!app.init(argc, argv)) {
        app.run();
    }
    return 0;
}