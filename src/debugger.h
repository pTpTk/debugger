#pragma once

#include <string>
#include <sstream>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <vector>
#include <iostream>
#include <cstdint>
#include <unordered_map>

#include "linenoise/linenoise.h"
#include "breakpoint.h"

class Debugger
{
  public:
    Debugger(char* program, pid_t pid) 
    : name{program}, pid{pid} {}

    void run();
    void set_breakpoint(std::intptr_t addr);

  private:
    std::string name;
    pid_t pid;
    std::unordered_map<std::intptr_t,Breakpoint> breakpoints;

    void handle_command(const std::string& line);
    void continue_execution();
};