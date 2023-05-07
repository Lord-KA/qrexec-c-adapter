#include <iostream>
#include <stdio.h>
#include <cassert>
#include <unistd.h>

#include "qrexec-client.h"

int main()
{
    exec_connector("RPCTest", "test.Add");
    /*
     * You can replace stdin/stdout with FDs or use them
     * directly (and save on buffers).
     */
    dup2(IN_FD, 0);
    dup2(OUT_FD, 1);

    int a = 1;
    int b = 2;
    int c = -1;

    std::cout << a << " " << b << std::endl;
    std::cout.flush();

    std::cin >> c;  /* c = a + b */
    assert(c == 3);
}
