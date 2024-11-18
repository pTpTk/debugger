#pragma once

#include <string>
#include <sstream>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <vector>
#include <iostream>

#include "linenoise/linenoise.h"

class Debugger
{
  public:
    Debugger(char* program, pid_t pid) 
    : program_name{program}, program_pid{pid} {}

    void run();

  private:
    std::string program_name;
    pid_t program_pid;

    void handle_command(const std::string& line);
    void continue_execution();
};