// c/c++ stuff
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

// unix stuff
#include <dirent.h>
#include <unistd.h>

// my stuff
#include <system.hpp>

static bool is_process(struct dirent* file) {
    if (file->d_type != DT_DIR)
        return false;

    // not efficient
    size_t len = strlen(file->d_name);
    char* name = file->d_name;
    if (std::all_of(name, name + len, isdigit))
        return true;

    return false;
}

namespace Sys {
System sys = {};
}

using namespace Sys;

bool Process::func() {
    return this->_functional;
}

bool Process::update() {
    std::ifstream stat_file(this->_stat_path);
    std::ifstream statm_file(this->_statm_path);

    if (!stat_file.is_open() || !statm_file.is_open())
        return false;

    stat_file >> this->_stat.pid;
    // i lov u c++ never change
    this->_stat.name = "";
    while (!stat_file.eof() && !stat_file.bad()) {
        char buf = stat_file.get();
        if (buf == ')')
            break;
        if (buf != '(')
            this->_stat.name += buf;
    }

    stat_file >> this->_stat.state >> this->_stat.parent_pid >>
        this->_stat.group_id;

    statm_file >> this->_statm.size >> this->_statm.resident >>
        this->_statm.shared >> this->_statm.text >> this->_statm.lib >>
        this->_statm.data >> this->_statm.dt;

    return true;
}

Process::Process(char* pid) {
    this->_stat_path  = std::string("/proc/") + pid + "/stat";
    this->_statm_path = std::string("/proc/") + pid + "/statm";
    this->update();
}

std::istream& operator>>(std::istream& is, ProcStat::Cpu& cpu) {
    is >> cpu.name >> cpu.user >> cpu.nice >> cpu.system >> cpu.idle >>
        cpu.iowait >> cpu.irq >> cpu.softirq >> cpu.steal >> cpu.guest >>
        cpu.guest_nice;
    return is;
}

ProcStat::ProcStat() {
    this->update();
}

std::vector<ProcStat::Cpu>& ProcStat::get_cpus() {
    return this->_curr_cpu;
}

std::vector<ProcStat::Cpu> ProcStat::get_cpus_diff() {
    std::vector<ProcStat::Cpu> dif(this->_curr_cpu.size());
    for (size_t i = 0; i < _curr_cpu.size(); ++i) {
        dif[i].name   = this->_prev_cpu[i].name;
        dif[i].user   = this->_prev_cpu[i].user - this->_curr_cpu[i].user;
        dif[i].nice   = this->_prev_cpu[i].nice - this->_curr_cpu[i].nice;
        dif[i].system = this->_prev_cpu[i].system - this->_curr_cpu[i].system;
        dif[i].idle   = this->_prev_cpu[i].idle - this->_curr_cpu[i].idle;
    }

    return this->_curr_cpu;
}

void ProcStat::update() {
    size_t cpu_count = 0;
    // hacky to do it
    DIR* dir = opendir("/dev/cpu");
    while (readdir(dir)) cpu_count++;
    closedir(dir);

    if (cpu_count == 0) {
        throw(std::runtime_error("cpu number too low!"));
    }

    if (cpu_count <= 2)
        return;
    cpu_count -= 1;

    this->_prev_cpu = this->_curr_cpu;
    this->_curr_cpu.resize(cpu_count, {});

    std::ifstream stat("/proc/stat");
    for (size_t i = 0; i < cpu_count; ++i) {
        stat >> this->_curr_cpu[i];
    }
}

std::vector<Process>& System::get_processes() {
    DIR* dir     = opendir("/proc");
    size_t tries = 1;
    while (dir == nullptr) dir = opendir("/proc");
    if (dir == nullptr)
        throw std::runtime_error("could not open /proc!?");

    struct dirent* dirent = nullptr;

    this->_process.clear();
    while ((dirent = readdir(dir))) {
        if (!is_process(dirent))
            continue;

        int pid   = std::stoi(dirent->d_name);
        auto proc = std::find_if(this->_process.begin(), this->_process.end(),
            [&](Process& x) { return x._stat.pid == pid; });

        if (proc != this->_process.end()) {
            bool alive = proc->update();
            if (!alive)
                this->_process.erase(proc);
        }
        else
            this->_process.push_back(Process(dirent->d_name));
    }
    closedir(dir);

    return this->_process;
}

#include <iostream>
#include <map>

void System::update() {
    // lmaooo should have not implemented it like this
    this->get_processes();
    this->stat.update();

    std::ifstream file("/proc/meminfo");
    std::string name, type;
    std::map<std::string, size_t> map;
    size_t len;
    while (file >> name >> len >> type) map[name] = len;

    this->_max_mem    = map["MemTotal:"];
    this->_free_mem   = map["MemFree:"];
    this->_av_mem     = map["MemAvailable:"];
    this->_cached_mem = map["Cached:"];
    this->_buffer_mem = map["Buffers:"];
}

System::System() {
    this->update();
}
