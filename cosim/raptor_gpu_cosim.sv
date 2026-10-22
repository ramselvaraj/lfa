`timescale 1ns/1ps
// Co-simulation top: the real RAPTOR-SoC register block, program memory and GPU
// core. The shared RAM is external (served by the C++ harness), and the CPU's
// AXI4-Lite master is driven by the harness on behalf of the QEMU guest.
module raptor_gpu_cosim (
    input  logic        clk,
    input  logic        reset,

    input  logic [31:0] s_axi_awaddr,
    input  logic        s_axi_awvalid,
    output logic        s_axi_awready,
    input  logic [31:0] s_axi_wdata,
    input  logic [3:0]  s_axi_wstrb,
    input  logic        s_axi_wvalid,
    output logic        s_axi_wready,
    output logic [1:0]  s_axi_bresp,
    output logic        s_axi_bvalid,
    input  logic        s_axi_bready,
    input  logic [31:0] s_axi_araddr,
    input  logic        s_axi_arvalid,
    output logic        s_axi_arready,
    output logic [31:0] s_axi_rdata,
    output logic [1:0]  s_axi_rresp,
    output logic        s_axi_rvalid,
    input  logic        s_axi_rready,

    // GPU program memory write port
    input  logic        prog_we,
    input  logic [9:0]  prog_waddr,
    input  logic [15:0] prog_wdata,

    // Shared RAM (word addressed, combinational read, single-cycle)
    output logic        mem_req,
    output logic        mem_we,
    output logic [31:0] mem_addr,
    output logic [31:0] mem_wdata,
    input  logic [31:0] mem_rdata
);
    logic        gpu_start, gpu_busy, gpu_done;
    logic [31:0] gpu_threads, gpu_src, gpu_dst, gpu_prog_base;
    logic [15:0] gpu_imem_addr, gpu_imem_rdata;

    gpu_mmio mmio (
        .command_ready(1'b1),
        .clk(clk), .reset(reset),
        .s_axi_awaddr(s_axi_awaddr), .s_axi_awvalid(s_axi_awvalid), .s_axi_awready(s_axi_awready),
        .s_axi_wdata(s_axi_wdata), .s_axi_wstrb(s_axi_wstrb), .s_axi_wvalid(s_axi_wvalid), .s_axi_wready(s_axi_wready),
        .s_axi_bresp(s_axi_bresp), .s_axi_bvalid(s_axi_bvalid), .s_axi_bready(s_axi_bready),
        .s_axi_araddr(s_axi_araddr), .s_axi_arvalid(s_axi_arvalid), .s_axi_arready(s_axi_arready),
        .s_axi_rdata(s_axi_rdata), .s_axi_rresp(s_axi_rresp), .s_axi_rvalid(s_axi_rvalid), .s_axi_rready(s_axi_rready),
        .gpu_busy(gpu_busy), .gpu_done(gpu_done), .start(gpu_start),
        .thread_count(gpu_threads), .src_addr(gpu_src), .dst_addr(gpu_dst),
        .prog_base(gpu_prog_base)
    );

    program_memory program_mem (
        .clk(clk),
        .write_enable(prog_we), .write_addr(prog_waddr), .write_data(prog_wdata),
        .prog_base(gpu_prog_base), .imem_addr(gpu_imem_addr), .imem_rdata(gpu_imem_rdata)
    );

    simt_gpu gpu (
        .clk(clk), .reset(reset), .start(gpu_start),
        .thread_count(gpu_threads), .src_addr(gpu_src), .dst_addr(gpu_dst),
        .busy(gpu_busy), .done(gpu_done),
        .imem_addr(gpu_imem_addr), .imem_rdata(gpu_imem_rdata),
        .mem_req(mem_req), .mem_we(mem_we),
        .mem_addr(mem_addr), .mem_wdata(mem_wdata),
        .mem_rdata(mem_rdata), .mem_ready(1'b1)
    );
endmodule
