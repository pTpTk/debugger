#pragma once

#include <unordered_map>
#include <string>
#include <cstdint>
#include <sys/ptrace.h>
#include <iostream>

class Regfile
{
  public:
    Regfile(char* program, pid_t pid)
    : name{program}, pid{pid} {}

    void reg_dump();
    void reg_read(const std::string& reg);
    void reg_write(const std::string& reg, uint64_t val);
    std::intptr_t get_pc();

  private:
    std::string name;
    pid_t pid;
    uint64_t regs[27];

    void reg_get();
};

