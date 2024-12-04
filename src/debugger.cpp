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


Debugger::Debugger(char* program, pid_t pid) 
    : name{program}, pid{pid}, regfile(program, pid) {

    auto fd = open(program, O_RDONLY);

    m_elf = elf::elf{elf::create_mmap_loader(fd)};
    m_dwarf = dwarf::dwarf{dwarf::elf::create_loader(m_elf)};

}

void Debugger::wait_for_signal() {
    int wait_status;
    waitpid(pid, &wait_status, 0);

    siginfo_t siginfo;
    ptrace(PTRACE_GETSIGINFO, pid, nullptr, &siginfo);

    switch(siginfo.si_signo) {
        case SIGTRAP:
            handle_sigtrap(siginfo);
            break;
        case SIGSEGV:
            std::cout << "Segfault. Reason: " << siginfo.si_code << std::endl;
            break;
        default:
            std::cout << "Got signal " << strsignal(siginfo.si_signo) << std::endl;
    }
}

void Debugger::handle_sigtrap(siginfo_t info) {
    switch (info.si_code) {
    //one of these will be set if a breakpoint was hit
    case SI_KERNEL:
    case TRAP_BRKPT:
    {
        set_pc(get_pc()-1); //put the pc back where it should be
        std::cout << "Hit breakpoint at address 0x" << std::hex << get_pc() << std::endl;
        auto offset_pc = offset_load_addr(get_pc()); //rember to offset the pc for querying DWARF
        auto line_entry = get_line_entry_from_pc(offset_pc);
        print_source(line_entry->file->path, line_entry->line);
        return;
    }
    //this will be set if the signal was sent by single stepping
    case TRAP_TRACE:
        return;
    default:
        std::cout << "Unknown SIGTRAP code " << info.si_code << std::endl;
        return;
    }
}

void Debugger::initialize_load_addr() {
    if(m_elf.get_hdr().type == elf::et::dyn) {
        std::ifstream map("/proc/" + std::to_string(pid) + "/maps");

        std::string addr;
        std::getline(map, addr, '-');
        addr = "0x" + addr;

        load_addr = std::stol(addr, 0, 16);
    }
}

uint64_t Debugger::offset_load_addr(uint64_t addr) {
   return addr - load_addr;
}

void Debugger::run() {
    wait_for_signal();
    initialize_load_addr();

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
    else if(is_abbrev(command, "step")) {
        step_in();
    }
    else if(is_abbrev(command, "stepi")) {
        step_inst();
        auto line_entry = get_line_entry_from_pc(get_pc());
        print_source(line_entry->file->path, line_entry->line);
    }
    else {
        std::cerr << "Unknown command\n";
    }
}

std::intptr_t Debugger::get_pc() {
    return regfile.get_pc();
}

std::intptr_t Debugger::get_offset_pc() {
    return offset_load_addr(get_pc());
}

void Debugger::set_pc(std::intptr_t pc) {
    reg_write("rip", pc);
}

void Debugger::continue_execution() {
    if(auto it = breakpoints.find(get_pc()); it != breakpoints.end()) {
        if(it->second.is_enabled()) {
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

dwarf::die Debugger::get_function_from_pc(uint64_t pc) {
    for(auto& cu : m_dwarf.compilation_units()) {
        if(dwarf::die_pc_range(cu.root()).contains(pc)) {
            for(const auto& die : cu.root()) {
                if(die.tag == dwarf::DW_TAG::subprogram) {
                    if(dwarf::die_pc_range(die).contains(pc)) {
                        return die;
                    }
                }
            }
        }
    }

    std::cerr << "can't find function at pc 0x" << std::hex << pc << std::endl;
    assert(false);
}

dwarf::line_table::iterator Debugger::get_line_entry_from_pc(uint64_t pc) {
    for(auto& cu : m_dwarf.compilation_units()) {
        if(dwarf::die_pc_range(cu.root()).contains(pc)) {
            auto& lt = cu.get_line_table();
            auto it = lt.find_address(pc);
            if (it == lt.end()) {
                std::cerr << "can't find line entry" << std::endl;
                assert(false);
            }
            else {
                return it;
            }
        }
    }

    std::cerr << "can't find line entry" << std::endl;
    assert(false);
}

void Debugger::print_source(const std::string& file_name, unsigned line, unsigned n_lines_context) {
    std::ifstream file {file_name};

    //Work out a window around the desired line
    auto start_line = line <= n_lines_context ? 1 : line - n_lines_context;
    auto end_line = line + n_lines_context + (line < n_lines_context ? n_lines_context - line : 0) + 1;

    char c{};
    auto current_line = 1u;
    //Skip lines up until start_line
    while (current_line != start_line && file.get(c)) {
        if (c == '\n') {
            ++current_line;
        }
    }

    //Output cursor if we're at the current line
    std::cout << (current_line==line ? "> " : "  ");

    //Write lines up until end_line
    while (current_line <= end_line && file.get(c)) {
        std::cout << c;
        if (c == '\n') {
            ++current_line;
            //Output cursor if we're at the current line
            std::cout << (current_line==line ? "> " : "  ");
        }
    }

    //Write newline and make sure that the stream is flushed properly
    std::cout << std::endl;
}

void Debugger::single_step() {
    ptrace(PTRACE_SINGLESTEP, pid, nullptr, nullptr);
    wait_for_signal();
}

void Debugger::step_inst() {
    auto it = breakpoints.find(get_pc());

    // regular case: no breakpoint set on inst
    if(it == breakpoints.end()) {
        single_step();
        return;
    }
    
    // breakpoint set on inst
    if(it->second.is_enabled()) {
        it->second.disable();       // restore inst
        ptrace(PTRACE_SINGLESTEP, pid, nullptr, nullptr);

        wait_for_signal();

        it->second.enable();
    }
}

void Debugger::step_in() {
    auto line = get_line_entry_from_pc(get_offset_pc())->line;

    while (get_line_entry_from_pc(get_offset_pc())->line == line) {
        step_inst();
    }

    auto line_entry = get_line_entry_from_pc(get_offset_pc());
    print_source(line_entry->file->path, line_entry->line);
}


