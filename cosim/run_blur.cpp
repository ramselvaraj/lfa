// Standalone co-sim check: run assembled GPU kernels on the Verilated RTL.
//   run_blur frame.hex out.hex src dst  prog1.hex threads1  [prog2.hex threads2 ...]
// Every kernel is loaded through the program port and launched through the real
// MMIO register block; completion is found by polling STATUS like the driver will.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include "gpu_harness.h"

static const size_t WORDS = 32768;

static std::vector<uint32_t> read_hex(const char *path) {
    std::ifstream f(path);
    if (!f) { fprintf(stderr, "cannot open %s\n", path); exit(2); }
    std::vector<uint32_t> v;
    std::string line;
    while (std::getline(f, line))
        if (!line.empty()) v.push_back(strtoul(line.c_str(), nullptr, 16));
    return v;
}

int main(int argc, char **argv) {
    if (argc < 7 || (argc - 5) % 2) {
        fprintf(stderr, "usage: %s frame.hex out.hex src dst prog.hex threads [...]\n", argv[0]);
        return 2;
    }
    uint32_t src = strtoul(argv[3], nullptr, 0), dst = strtoul(argv[4], nullptr, 0);
    std::vector<uint32_t> ram(WORDS, 0);
    auto frame = read_hex(argv[1]);
    for (size_t i = 0; i < frame.size(); i++) ram[src + i] = frame[i];
    GpuHarness gpu(ram.data(), WORDS);

    for (int a = 5; a < argc; a += 2) {
        auto words = read_hex(argv[a]);
        std::vector<uint16_t> prog(words.begin(), words.end());
        uint32_t threads = strtoul(argv[a + 1], nullptr, 0);
        gpu.load_program(prog);
        gpu.mmio_write(GpuHarness::REG_PROG_BASE, 0);
        gpu.mmio_write(GpuHarness::REG_THREADS, threads);
        gpu.mmio_write(GpuHarness::REG_SRC, src);
        gpu.mmio_write(GpuHarness::REG_DST, dst);
        uint64_t t0 = gpu.cycles();
        if (gpu.mmio_write(GpuHarness::REG_CTRL, 1) != 0) { fprintf(stderr, "start rejected\n"); return 1; }
        uint32_t st = 0;
        unsigned polls = 0;
        do { gpu.mmio_read(GpuHarness::REG_STATUS, &st); polls++; } while (!(st & 2) && polls < 50000000);
        if (!(st & 2)) { fprintf(stderr, "timeout waiting for done\n"); return 1; }
        printf("kernel %s: %u threads, %llu cycles, %u status polls\n", argv[a], threads,
               (unsigned long long)(gpu.cycles() - t0), polls);
    }

    FILE *o = fopen(argv[2], "w");
    for (size_t i = 0; i < frame.size(); i++) fprintf(o, "%08x\n", ram[dst + i] & 0x00ffffff);
    fclose(o);
    return 0;
}
