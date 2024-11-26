#pragma once

#include <string>
#include <sstream>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <vector>
#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <sys/user.h>
#include <fcntl.h>
#include <assert.h>
#include <fstream>

#include "linenoise/linenoise.h"
#include "breakpoint.h"
#include "regfile.h"
#include "libelfin/dwarf/dwarf++.hh"
#include "libelfin/elf/elf++.hh"

class Debugger
{
  public:
    Debugger(char* program, pid_t pid);

    void run();

  private:
    std::string name;
    pid_t pid;
    std::unordered_map<std::intptr_t,Breakpoint> breakpoints;
    Regfile regfile;
    elf::elf m_elf;
    dwarf::dwarf m_dwarf;
    std::intptr_t load_addr;

    void wait_for_signal();
    void handle_sigtrap(siginfo_t info);


    void initialize_load_addr();
    uint64_t offset_load_addr(uint64_t addr);
    
    void handle_command(const std::string& line);
    void continue_execution();

    // breakpoint
    void set_breakpoint(std::intptr_t addr);
    std::intptr_t get_pc();
    void set_pc(std::intptr_t pc);

    // register
    void reg_dump();
    void reg_read(const std::string& reg);
    void reg_write(const std::string& reg, uint64_t val);
    
    dwarf::die get_function_from_pc(uint64_t pc);
    dwarf::line_table::iterator get_line_entry_from_pc(uint64_t pc);
    void print_source(const std::string& file_name, unsigned line, unsigned n_lines_context = 0);
};
