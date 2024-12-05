#include "regfile.h"

std::unordered_map<std::string, reg> reg_names
{
    {"r15", reg::r15},
    {"r14", reg::r14},
    {"r13", reg::r13},
    {"r12", reg::r12},
    {"rbp", reg::rbp},
    {"rbx", reg::rbx},
    {"r11", reg::r11},
    {"r10", reg::r10},
    {"r9", reg::r9},
    {"r8", reg::r8},
    {"rax", reg::rax},
    {"rcx", reg::rcx},
    {"rdx", reg::rdx},
    {"rsi", reg::rsi},
    {"rdi", reg::rdi},
    {"orig_rax", reg::orig_rax},
    {"rip", reg::rip},
    {"cs", reg::cs},
    {"eflags", reg::eflags},
    {"rsp", reg::rsp},
    {"ss", reg::ss},
    {"fs_base", reg::fs_base},
    {"gs_base", reg::gs_base},
    {"ds", reg::ds},
    {"es", reg::es},
    {"fs", reg::fs},
    {"gs", reg::gs}
};

void Regfile::reg_get() {
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs);
}

void Regfile::reg_dump() {
    reg_get();

    std::cout << "r15       " << std::hex << regs[ 0] << std::endl;
    std::cout << "r14       " << std::hex << regs[ 1] << std::endl;
    std::cout << "r13       " << std::hex << regs[ 2] << std::endl;
    std::cout << "r12       " << std::hex << regs[ 3] << std::endl;
    std::cout << "rbp       " << std::hex << regs[ 4] << std::endl;
    std::cout << "rbx       " << std::hex << regs[ 5] << std::endl;
    std::cout << "r11       " << std::hex << regs[ 6] << std::endl;
    std::cout << "r10       " << std::hex << regs[ 7] << std::endl;
    std::cout << "r9        " << std::hex << regs[ 8] << std::endl;
    std::cout << "r8        " << std::hex << regs[ 9] << std::endl;
    std::cout << "rax       " << std::hex << regs[10] << std::endl;
    std::cout << "rcx       " << std::hex << regs[11] << std::endl;
    std::cout << "rdx       " << std::hex << regs[12] << std::endl;
    std::cout << "rsi       " << std::hex << regs[13] << std::endl;
    std::cout << "rdi       " << std::hex << regs[14] << std::endl;
    std::cout << "orig_rax  " << std::hex << regs[15] << std::endl;
    std::cout << "rip       " << std::hex << regs[16] << std::endl;
    std::cout << "cs        " << std::hex << regs[17] << std::endl;
    std::cout << "eflags    " << std::hex << regs[18] << std::endl;
    std::cout << "rsp       " << std::hex << regs[19] << std::endl;
    std::cout << "ss        " << std::hex << regs[20] << std::endl;
    std::cout << "fs_base   " << std::hex << regs[21] << std::endl;
    std::cout << "gs_base   " << std::hex << regs[22] << std::endl;
    std::cout << "ds        " << std::hex << regs[23] << std::endl;
    std::cout << "es        " << std::hex << regs[24] << std::endl;
    std::cout << "fs        " << std::hex << regs[25] << std::endl;
    std::cout << "gs        " << std::hex << regs[26] << std::endl;
}

void Regfile::reg_read(const std::string& reg) {
    reg_get();
    int index = (int)reg_names[reg];
    std::cout << reg << "\t0x" << std::hex << regs[index] << std::endl;
}

std::intptr_t Regfile::reg_read(const reg r) {
    reg_get();
    int index = (int)r;
    return regs[index];
}

void Regfile::reg_write(const std::string& reg, uint64_t val) {
    reg_get();
    int index = (int)reg_names[reg];
    regs[index] = val;
    ptrace(PTRACE_SETREGS, pid, nullptr, &regs);
}

std::intptr_t Regfile::get_pc() {
    reg_get();
    return regs[16];
}
