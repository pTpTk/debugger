#pragma once

#include <unordered_map>
#include <string>
#include <cstdint>
#include <sys/ptrace.h>
#include <iostream>

enum class reg
{
    rax, rbx, rcx, rdx,
    rdi, rsi, rbp, rsp,
    r8,  r9,  r10, r11,
    r12, r13, r14, r15,
    rip, rflags,    cs,
    orig_rax, fs_base,
    gs_base,
    fs, gs, ss, ds, es
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

