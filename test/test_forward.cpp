#include <iostream>
#include <utility>
#include <memory>
#include <string>

using namespace std;

//https://www.cnblogs.com/5iedu/p/7698710.html

struct A {
    A(int& n) {
        cout << "lvalue overload, n = " << n << endl;
    }

    A(int&& n) {
        cout << "rvalue overload, n = " << n << endl;
    }
};


class B {
public:
    template<typename T1, typename T2, typename T3>
    B(T1&& t1, T2&& t2, T3&& t3) 
    : a1(forward<T1>(t1)) 
    , a2(forward<T2>(t2))
    , a3(forward<T3>(t3)) {
    }
private:
    A a1, a2, a3;
};


void test_forward() {
    A a(1);
    int i = 2;
    A c(i);
    B(1, i, 2);
}

int main(int argc, char** argv) {

    test_forward();
    return 0;
}