#include "breakpoint.h"

void Breakpoint::enable() {
    auto data = ptrace(PTRACE_PEEKDATA, pid, addr, nullptr);
    old_inst = (uint8_t)(data & 0xff);
    uint64_t int3 = 0xcc;
    uint64_t new_inst = ((data & ~0xff) | int3);
    ptrace(PTRACE_POKEDATA, pid, addr, new_inst);

    enabled = true;
}

void Breakpoint::disable() {
    auto data = ptrace(PTRACE_PEEKDATA, pid, addr, nullptr);
    auto restored_data = ((data & ~0xff) | old_inst);
    ptrace(PTRACE_POKEDATA, pid, addr, restored_data);

    enabled = false;
}