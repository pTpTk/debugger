#include <unistd.h>
#include <cassert>
#include <iostream>
#include <sys/ptrace.h>
#include <sys/personality.h>

#include "debugger.h"

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cerr << "usage: debug <program_name>\n";
        return -1;
    }

    char* program = argv[1];

    auto pid = fork();

    assert(pid >= 0);

    if(pid) { // parent process (debugger)
        std::cout << "child pid: " << pid << std::endl;

        Debugger dbg(program, pid);
        dbg.run();
    }
    else { // child process (debugee)
        personality(ADDR_NO_RANDOMIZE);
        ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
        execl(program, program, nullptr);

    }
    return 0;
}