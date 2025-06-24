#ifndef __SYSTEM_SYSTEM_HPP__
#define __SYSTEM_SYSTEM_HPP__

#include <cstdint>

#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <sys/types.h>
#include <unistd.h>

namespace sys {

class System;
class Process;

class Process {
public:
    struct _proc_stat {
        pid_t pid;
        pid_t parent_pid;
        pid_t group_id;
        char state;
        std::string name;
    };

    struct _proc_statm {
        size_t size;
        size_t resident;
        size_t shared;
        size_t text;
        size_t lib;
        size_t data;
        size_t dt;
    };

    std::string _cmd;

    _proc_stat _stat;
    _proc_statm _statm;

    pid_t _pid;
    std::ifstream _stat_file;
    std::ifstream _statm_file;
    std::ifstream _cmd_file;
    bool _functional = false;

    Process();
    Process(char* pid);
    Process(pid_t pid);

    Process(const Process& other) = delete;
    Process(Process&& other);

    Process& operator=(const Process& other) = delete;
    Process& operator=(Process&& other);

    bool update();
    bool is_kernel() const;
    bool func();

    /** total ram usage */
    uint64_t total() const {
        long page_size = sysconf(_SC_PAGESIZE);
        return page_size * this->_statm.resident;
    }

    friend class System;
};

class ProcStat {
public:
    struct Cpu {
        std::string name;
        size_t user;
        size_t nice;
        size_t system;
        size_t idle;
        size_t iowait;
        size_t irq;
        size_t softirq;
        size_t steal;
        size_t guest;
        size_t guest_nice;
    };

private:
    std::vector<Cpu> _curr_cpu;
    std::vector<Cpu> _prev_cpu;

public:
    ProcStat();
    ~ProcStat() {}
    std::vector<Cpu>& get_cpus();
    std::vector<Cpu> get_cpus_diff();

    void update();

    friend class System;
};

using Processes = std::unordered_map<size_t, Process>;

class System {
private:
    Processes _process;

public:
    ProcStat stat;

    uint64_t _max_mem;
    uint64_t _free_mem;
    uint64_t _av_mem;

    uint64_t _cached_mem;
    uint64_t _buffer_mem;

    const Processes& get_processes() const;
    System();

    void update();
};

extern System sys;

} // namespace sys

#endif // __PROCESS_HPP__
