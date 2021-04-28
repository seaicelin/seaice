#include "test.h"
#include <iostream>

using namespace std;

#pragma pack(1) //按一个字节对齐
struct WSFrameHead {
    uint32_t opcode : 4;
    bool fin: 1;
    bool rsv1: 1;
    bool rsv2: 1;
    bool rsv3: 1;
    uint32_t payload_len: 7;
    bool mask: 1;
};
#pragma pack()

#pragma pack(1)
struct bs {
    int a : 8;
    int b : 2;
    int c : 6;
}data;
#pragma pack()

void test_ws_header() {
    cout << "ws frame size = " << sizeof(WSFrameHead) << endl;
    cout << "bs size = " << sizeof(bs) << endl;
}

int main()
{

    test_ws_header();

    return 0;
}
