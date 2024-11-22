#pragma once

#include <string>
#include <sstream>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <vector>
#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <sys/user.h>

#include "linenoise/linenoise.h"
#include "breakpoint.h"
#include "regfile.h"

class Debugger
{
  public:
    Debugger(char* program, pid_t pid) 
    : name{program}, pid{pid}, regfile(program, pid) {}

    void run();

  private:
    std::string name;
    pid_t pid;
    std::unordered_map<std::intptr_t,Breakpoint> breakpoints;
    Regfile regfile;

    void wait_for_signal();

    void handle_command(const std::string& line);
    void continue_execution();

    void set_breakpoint(std::intptr_t addr);
    void reg_dump();
    void reg_read(const std::string& reg);
    void reg_write(const std::string& reg, uint64_t val);
};
