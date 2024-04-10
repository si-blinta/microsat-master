HOST-CC           = gcc
HOST-CFLAGS       = -std=gnu99
HOST-LIBS         = `dpu-pkg-config --cflags --libs dpu`
DPU-CC            = dpu-upmem-dpurte-clang
NB_TASKLETS       ?= 1
clean:
	@rm -f bin/host bin/dpu
	@echo "Cleaned"
all: clean dpu host microsat

dpu: src/dpu.c src/microsat.c 
	@$(DPU-CC) -DNR_TASKLETS=$(NB_TASKLETS) -DDPU -o bin/dpu src/dpu.c src/microsat.c 
	@echo "Compiled DPU with $(NB_TASKLETS) Tasklets"

host: src/host.c src/microsat.c src/hostTools.c
	@$(HOST-CC) $(HOST-CFLAGS) src/host.c src/microsat.c src/hostTools.c -o bin/host $(HOST-LIBS)
	@echo "Compiled HOST"
	