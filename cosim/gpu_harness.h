// Drives the Verilated RAPTOR GPU: AXI4-Lite master, program load port, and a
// word-addressed shared RAM that the GPU core reads and writes.
#pragma once
#include <cstdint>
#include <vector>
#include "Vraptor_gpu_cosim.h"

class GpuHarness {
public:
    static constexpr uint32_t MMIO_BASE = 0x40000000u;
    static constexpr uint32_t REG_CTRL = 0x00, REG_STATUS = 0x04, REG_THREADS = 0x08,
                              REG_SRC = 0x0c, REG_DST = 0x10, REG_PROG_BASE = 0x14;

    // ram: word-addressed backing store, `words` long (not owned).
    GpuHarness(uint32_t *ram, size_t words);
    ~GpuHarness();

    void reset();
    void load_program(const std::vector<uint16_t> &prog, uint32_t at = 0);
    // Returns the AXI response code (0 = OKAY, 2 = SLVERR).
    int  mmio_write(uint32_t off, uint32_t data, uint8_t strb = 0xf);
    int  mmio_read(uint32_t off, uint32_t *data);
    // Advance the clock; returns cycles consumed.
    void tick(unsigned n = 1);
    uint64_t cycles() const { return cycles_; }

private:
    void settle();
    void one_cycle();
    Vraptor_gpu_cosim *top_;
    uint32_t *ram_;
    size_t words_;
    uint64_t cycles_ = 0;
};
