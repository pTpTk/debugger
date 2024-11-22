#include "debugger.h"

namespace {
    std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> out{};
        std::stringstream ss{s};
        std::string item;
        
        while(std::getline(ss, item, delim)) {
            out.push_back(item);
        }

        return out;
    }

    bool is_abbrev(const std::string& s, const std::string& ref) {
        if(s.size() > ref.size()) return false;

        return std::equal(s.begin(), s.end(), ref.begin());
    }
}


void Debugger::wait_for_signal() {
    int wait_status;
    waitpid(pid, &wait_status, 0);
}

void Debugger::run() {
    wait_for_signal();

    char* line = nullptr;
    while((line = linenoise("dbg> ")) != nullptr) {
        handle_command(line);
        linenoiseHistoryAdd(line);
        linenoiseFree(line);
    }
}

void Debugger::handle_command(const std::string& line) {
    auto args = split(line, ' ');
    auto command = args[0];

    if(is_abbrev(command, "continue")) {
        continue_execution();
    }
    else if(is_abbrev(command, "break")) {
        std::string addr {args[1], 2}; // naively assume that the user has written 0xADDRESS
        set_breakpoint(std::stol(addr, 0, 16));
    }
    else if(is_abbrev(command, "register")) {
        if (is_abbrev(args[1], "dump")) {
            reg_dump();
        }
        else if (is_abbrev(args[1], "read")) {
            reg_read(args[2]);
        }
        else if (is_abbrev(args[1], "write")) {
            std::string val {args[3], 2}; //assume 0xVAL
            reg_write(args[2], std::stol(val, 0, 16));
        }
    }
    else {
        std::cerr << "Unknown command\n";
    }
}

void Debugger::continue_execution() {

    std:intptr_t prev_pc = regfile.get_pc() - 1;

    if(auto it = breakpoints.find(prev_pc); it != breakpoints.end()) {
        if(it->second.is_enabled()) {
            reg_write("rip", prev_pc);  // back up pc
            it->second.disable();       // restore inst
            ptrace(PTRACE_SINGLESTEP, pid, nullptr, nullptr);

            wait_for_signal();

            it->second.enable();
        }
    }

    ptrace(PTRACE_CONT, pid, nullptr, nullptr);

    wait_for_signal();

}

void Debugger::set_breakpoint(std::intptr_t addr) {
    std::cout << "Set breakpoint at address 0x" << std::hex << addr << std::endl;
    Breakpoint bp{pid, addr};
    bp.enable();
    breakpoints[addr] = bp;
}

void Debugger::reg_dump() {
    regfile.reg_dump();
}

void Debugger::reg_read(const std::string& reg) {
    regfile.reg_read(reg);
}

void Debugger::reg_write(const std::string& reg, uint64_t val) {
    regfile.reg_write(reg, val);
}
