#copyright by:
#George Washington University
#Hao Howie Huang
#howie@gwu.edu

exe = bfs

cc = "$(shell which nvcc)" 
#flags = -I. -fopenmp -march=athlon64 -O3
flags = -I. #-fopenmp -O3
#flags += -std=c++11

ifeq ($(debug), 1)
	flags+= -DDEBUG 
endif

objs = $(patsubst %.cu,%.o,$(wildcard ../../lib/*.cu))\
			$(patsubst %.cu,%.o,$(wildcard *.cu))

deps = $(wildcard ../../lib/*.h) \
				$(wildcard *.h) \
				Makefile

%.o:%.cu $(deps)
	$(cc) -c $< -o $@ $(flags)

$(exe):$(objs)
	$(cc) $(objs) -o $(exe) $(flags)

test:$(exe)
	./bfs /usr3/graduate/rzhan/graphs/livejournal/fw_begin.bin /usr3/graduate/rzhan/graphs/livejournal/fw_adjacent.bin /usr3/graduate/rzhan/graphs/livejournal/bw_begin.bin /usr3/graduate/rzhan/graphs/livejournal/bw_adjacent.bin 

clean:
	rm -rf $(exe) $(objs) 
