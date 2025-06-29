// c stuff
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// c++ stuff
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

// unix stuff
#include <dirent.h>
#include <unistd.h>

// my stuff
#include <system/system.hpp>

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

namespace sys {
System sys = {};
}

using namespace sys;

Process::Process() {}

Process::Process(char* pid)
    : Process(static_cast<pid_t>(std::stoi(pid))) {}

Process::Process(pid_t pid) : _pid(pid) {
    this->update();
}

bool Process::func() {
    return _functional;
}

bool Process::is_kernel() const {
    return !_cmd.empty() && _cmd.front() == '[' &&
           _cmd.back() == ']';
}

// TODO: rework this so constructor gets the static data
// and update the dynamic data
bool Process::update() {
    std::string base = "/proc/" + std::to_string(_pid);

    std::ifstream stat_file(base + "/stat");
    std::ifstream statm_file(base + "/statm");
    std::ifstream cmd_file(base + "/cmdline");

    if (!stat_file || !statm_file) {
        _functional = false;
        return false;
    }

    _functional = true;

    stat_file >> _stat.pid;
    _stat.name.clear();
    char c;
    while (stat_file.get(c)) {
        if (c == ')')
            break;
        if (c != '(')
            _stat.name += c;
    }

    stat_file >> _stat.state >> _stat.parent_pid >>
        _stat.group_id;

    statm_file >> _statm.size >> _statm.resident >>
        _statm.shared >> _statm.text >> _statm.lib >>
        _statm.data >> _statm.dt;

    _cmd.clear();
    if (cmd_file) {
        while (cmd_file.get(c))
            _cmd += (c == '\0') ? ' ' : c;

        if (!_cmd.empty() && _cmd.back() == ' ')
            _cmd.pop_back();

        if (_cmd.empty())
            _cmd = "[" + _stat.name + "]";
    }
    else {
        _cmd = "[" + _stat.name + "]";
    }

    return true;
}

std::istream& operator>>(
    std::istream& is, ProcStat::Cpu& cpu) {
    is >> cpu.name >> cpu.user >> cpu.nice >> cpu.system >>
        cpu.idle >> cpu.iowait >> cpu.irq >> cpu.softirq >>
        cpu.steal >> cpu.guest >> cpu.guest_nice;
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
        dif[i].name = this->_prev_cpu[i].name;
        dif[i].user = this->_prev_cpu[i].user -
                      this->_curr_cpu[i].user;
        dif[i].nice = this->_prev_cpu[i].nice -
                      this->_curr_cpu[i].nice;
        dif[i].system = this->_prev_cpu[i].system -
                        this->_curr_cpu[i].system;
        dif[i].idle = this->_prev_cpu[i].idle -
                      this->_curr_cpu[i].idle;
    }

    return this->_curr_cpu;
}

void ProcStat::update() {
    size_t cpu_count = 0;
    // hacky to do it
    DIR* dir = opendir("/dev/cpu");
    if (dir == NULL) {
        return;
    }
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

const Processes& System::get_processes() const {
    return this->_process;
}

void System::update() {
    DIR* dir = opendir("/proc");
    while (dir == nullptr) dir = opendir("/proc");
    if (dir == nullptr)
        throw std::runtime_error("could not open /proc!?");

    struct dirent* dirent = nullptr;

    std::vector<int> pids = {};
    while ((dirent = readdir(dir))) {
        if (!is_process(dirent))
            continue;

        int pid = std::stoi(dirent->d_name);
        pids.push_back(pid);
    }
    closedir(dir);

    // update all loaded pids
    for (auto& entry : this->_process) {
        auto pos = std::find(
            pids.begin(), pids.end(), entry.first);

        if (pos != pids.end()) {
            bool ok = entry.second.update();
            if (!ok)
                this->_process.erase(entry.first);
            pids.erase(pos);
        }
        else
            this->_process.erase(entry.first);
    }

    // create pids
    for (const auto& pid : pids) {
        this->_process[pid] = sys::Process(pid);
    }

    this->stat.update();

    std::ifstream file("/proc/meminfo");
    std::string name, type;
    std::unordered_map<std::string, size_t> map;
    size_t len;
    while (file >> name >> len >> type) map[name] = len;

    this->_max_mem    = 1000 * map["MemTotal:"];
    this->_free_mem   = 1000 * map["MemFree:"];
    this->_av_mem     = 1000 * map["MemAvailable:"];
    this->_cached_mem = 1000 * map["Cached:"];
    this->_buffer_mem = 1000 * map["Buffers:"];
}

System::System() {
    this->update();
}
