RTL_ROOT  := ../rtl/RAPTOR-SoC
GPU_RTL   := $(addprefix $(RTL_ROOT)/simt_gpu/rtl/,simt_gpu.sv warp.sv regfile.sv alu.sv decoder.sv dcr.sv dispatcher.sv scheduler.sv mem_unit.sv) \
             $(RTL_ROOT)/soc/rtl/gpu_mmio.sv $(RTL_ROOT)/soc/rtl/program_memory.sv
BUILD     := build
VERILATOR ?= verilator

$(BUILD)/run_blur: raptor_gpu_cosim.sv $(GPU_RTL) gpu_harness.cpp gpu_harness.h run_blur.cpp
	mkdir -p $(BUILD)/obj
	$(VERILATOR) --cc --exe --build -O2 -Wno-fatal --top-module raptor_gpu_cosim \
	  -Mdir $(BUILD)/obj -o ../run_blur raptor_gpu_cosim.sv $(GPU_RTL) gpu_harness.cpp run_blur.cpp

clean:
	rm -rf $(BUILD)
