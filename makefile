HOST-CC           = gcc
HOST-CFLAGS       = -std=gnu99
HOST-LIBS         = `dpu-pkg-config --cflags --libs dpu`
DPU-CC            = dpu-upmem-dpurte-clang
NB_TASKLETS       ?= 1
all: clean dpu host

local: src/host.c src/microsat.c src/log.c 
	@gcc src/host.c src/microsat.c src/log.c  $(HOST-CFLAGS) -o bin/local -lm -g

clean:
	@rm -f bin/host bin/dpu bin/local
	@echo "Cleaned"

dpu: src/dpu.c src/microsat.c src/utils.c
	@$(DPU-CC) -DNR_TASKLETS=$(NB_TASKLETS) -DDPU -o bin/dpu src/dpu.c src/microsat.c src/utils.c -g
	@echo "Compiled DPU with $(NB_TASKLETS) Tasklets"

host: src/host.c src/microsat.c src/hostTools.c src/log.c src/utils.c
	@$(HOST-CC) $(HOST-CFLAGS) src/host.c src/microsat.c src/hostTools.c src/log.c src/utils.c -o bin/host $(HOST-LIBS)
	@echo "Compiled HOST"
	