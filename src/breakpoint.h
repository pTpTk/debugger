#pragma once 

#include <unistd.h>
#include <cstdint>
#include <sys/ptrace.h>

class Breakpoint
{
  public:
    Breakpoint() {}
    Breakpoint(pid_t pid, std::intptr_t addr)
    : pid{pid}, addr{addr}, enabled{false} {}

    void enable();
    void disable();

    bool is_enabled() const { return enabled; }
    std::intptr_t get_addr() const { return addr; }

  private:
    pid_t pid;
    std::intptr_t addr;
    bool enabled;
    uint8_t old_inst;
};