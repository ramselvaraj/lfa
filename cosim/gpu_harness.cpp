#include "gpu_harness.h"
#include <cstdio>
#include <cstdlib>

GpuHarness::GpuHarness(uint32_t *ram, size_t words)
    : top_(new Vraptor_gpu_cosim), ram_(ram), words_(words) {
    reset();
}

GpuHarness::~GpuHarness() { top_->final(); delete top_; }

// Combinational memory: present read data for the current address, then let the
// design re-evaluate so anything downstream of mem_rdata settles.
void GpuHarness::settle() {
    top_->eval();
    top_->mem_rdata = ram_[top_->mem_addr % words_];
    top_->eval();
}

// One full clock period. Pre-edge outputs are what the design sampled at the
// rising edge, so a write is committed to RAM from the values latched here.
void GpuHarness::one_cycle() {
    top_->clk = 0;
    settle();
    bool wr = top_->mem_req && top_->mem_we;
    uint32_t waddr = top_->mem_addr % words_, wdata = top_->mem_wdata;
    top_->clk = 1;
    top_->eval();
    if (wr) ram_[waddr] = wdata;
    cycles_++;
}

void GpuHarness::tick(unsigned n) { while (n--) one_cycle(); }

void GpuHarness::reset() {
    top_->s_axi_awvalid = top_->s_axi_wvalid = top_->s_axi_arvalid = 0;
    top_->s_axi_bready = top_->s_axi_rready = 0;
    top_->s_axi_wstrb = 0xf;
    top_->prog_we = 0;
    top_->reset = 1;
    tick(4);
    top_->reset = 0;
    tick(2);
}

void GpuHarness::load_program(const std::vector<uint16_t> &prog, uint32_t at) {
    for (size_t i = 0; i < prog.size(); i++) {
        top_->prog_we = 1;
        top_->prog_waddr = (at + i) & 0x3ff;
        top_->prog_wdata = prog[i];
        tick();
    }
    top_->prog_we = 0;
    tick();
}

int GpuHarness::mmio_write(uint32_t off, uint32_t data, uint8_t strb) {
    top_->s_axi_awaddr = MMIO_BASE + off;
    top_->s_axi_wdata = data;
    top_->s_axi_wstrb = strb;
    top_->s_axi_awvalid = top_->s_axi_wvalid = 1;
    bool aw = false, w = false;
    for (int guard = 0; (!aw || !w) && guard < 1000; guard++) {
        // Sample ready before the edge: valid & ready at the edge is the handshake.
        top_->clk = 0;
        settle();
        bool awr = top_->s_axi_awready, wr = top_->s_axi_wready;
        one_cycle();
        if (awr && !aw) { aw = true; top_->s_axi_awvalid = 0; }
        if (wr && !w)   { w = true;  top_->s_axi_wvalid = 0; }
    }
    top_->s_axi_awvalid = top_->s_axi_wvalid = 0;
    top_->s_axi_bready = 1;
    int resp = -1;
    for (int guard = 0; guard < 1000; guard++) {
        top_->clk = 0;
        settle();
        if (top_->s_axi_bvalid) { resp = top_->s_axi_bresp; one_cycle(); break; }
        one_cycle();
    }
    top_->s_axi_bready = 0;
    tick();
    return resp;
}

int GpuHarness::mmio_read(uint32_t off, uint32_t *data) {
    top_->s_axi_araddr = MMIO_BASE + off;
    top_->s_axi_arvalid = 1;
    for (int guard = 0; guard < 1000; guard++) {
        top_->clk = 0;
        settle();
        bool arr = top_->s_axi_arready;
        one_cycle();
        if (arr) break;
    }
    top_->s_axi_arvalid = 0;
    top_->s_axi_rready = 1;
    int resp = -1;
    for (int guard = 0; guard < 1000; guard++) {
        top_->clk = 0;
        settle();
        if (top_->s_axi_rvalid) { *data = top_->s_axi_rdata; resp = top_->s_axi_rresp; one_cycle(); break; }
        one_cycle();
    }
    top_->s_axi_rready = 0;
    tick();
    return resp;
}
