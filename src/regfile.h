#pragma once

#include <unordered_map>
#include <string>
#include <cstdint>
#include <sys/ptrace.h>
#include <iostream>

enum class reg
{
    r15, 
    r14,
    r13,
    r12,
    rbp,
    rbx,
    r11,
    r10,
    r9,
    r8,
    rax,
    rcx,
    rdx,
    rsi,
    rdi,
    orig_rax,
    rip,
    cs,
    eflags,
    rsp,
    ss,
    fs_base,
    gs_base,
    ds,
    es,
    fs,
    gs
};

class Regfile
{
  public:
    Regfile(char* program, pid_t pid)
    : name{program}, pid{pid} {}

    void reg_dump();
    void reg_read(const std::string& reg);
    std::intptr_t reg_read(const reg r);
    void reg_write(const std::string& reg, uint64_t val);
    std::intptr_t get_pc();

  private:
    std::string name;
    pid_t pid;
    uint64_t regs[27];

    void reg_get();
};

