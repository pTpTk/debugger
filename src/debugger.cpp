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


void Debugger::run() {
    int wait_status;
    waitpid(program_pid, &wait_status, 0);

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
    else {
        std::cerr << "Unknown command\n";
    }
}

void Debugger::continue_execution() {
    ptrace(PTRACE_CONT, program_pid, nullptr, nullptr);

    int wait_status;
    auto options = 0;
    waitpid(program_pid, &wait_status, options);
}