# This is a -*- Makefile -*-

# Default target.
all:

ifdef TILERA_ROOT

#CC = $(TILERA_ROOT)/bin/tile-gcc
CC = $(TILERA_ROOT)/bin/tile-g++

else

ifeq ($(filter tile%,$(shell uname -m)),)
$(error The 'TILERA_ROOT' environment variable is not set.)
endif

CC = g++

endif

#CFLAGS = -g -O2
CFLAGS = -g
#CFLAGS = -g -O -frepo 
#CFLAGS = -g -std=gnu99 -O2

EXE = lookup
EXE2 = lookup2

Rib.o: Rib.cpp 
	$(CC) $(CFLAGS) $(XFLAGS) -c $< -pthread -o $@

Fib.o: Fib.cpp 
	$(CC) $(CFLAGS) $(XFLAGS) -c $< -pthread -o $@

lookup: main.cpp Rib.o Fib.o 
	$(CC) $(LDFLAGS) $(XFLAGS) $<  Rib.o Fib.o -pthread -ltmc -o $@ 

lookup2: main1.cpp Rib.o Fib.o 
	$(CC) $(LDFLAGS) $(XFLAGS) $<  Rib.o Fib.o -pthread -ltmc -o $@ 

all: $(EXE) $(EXE2) 

clean:
	rm -f *.o $(EXE)  $(EXE2) 

.PHONY: all clean


FILTER = sort | sed 's/tile [0-9]*:/tile N:/'

ifdef TILERA_ROOT

TILE_MONITOR = $(TILERA_ROOT)/bin/tile-monitor

COMMON_ARGS = \
  --mkdir /opt/test \
  --cd /opt/test \
  --upload $(EXE) $(EXE) \
  --upload rib.txt.port rib.txt.port \
  --mount-same /home/yt/ \
  --resume \
  --root \
  --tile 6x6 -- $(EXE) /home/yt/lookup/dip.txt rib.txt.port

DEV_ARGS = $(COMMON_ARGS)

# For faster execution, also specify the "--functional" option to run the
# simulator in functional mode, as opposed to the default timing-accurate mode.
SIM_ARGS = --image 4x4 $(COMMON_ARGS)

SINGLE_ARGS = \
  --mkdir /opt/test \
  --cd /opt/test \
  --upload $(EXE2) $(EXE2) \
  --upload rib.txt.port rib.txt.port \
  --mount-same /home/yt/ \
  --resume \
  --root \
  --tile 6x6 -- $(EXE2) /home/yt/lookup/dip.txt rib.txt.port

run_dev: $(EXE)
	$(TILE_MONITOR) $(DEV_ARGS)

many_run: $(EXE2)
	$(TILE_MONITOR) $(SINGLE_ARGS)

test_dev: $(EXE)
	set -e -o pipefail; \
	$(TILE_MONITOR) $(DEV_ARGS) \
	  | $(FILTER) > output.run; \
	cat output.txt | $(FILTER) | diff output.run -

run_sim: $(EXE)
	$(TILE_MONITOR) $(SIM_ARGS)

test_sim: $(EXE)
	set -e -o pipefail; \
	$(TILE_MONITOR) $(SIM_ARGS) \
	  | $(FILTER) > output.run; \
	cat output.txt | $(FILTER) | diff output.run -

.PHONY: run_sim test_sim run_dev test_dev single_run

else

run: $(EXE)
	./$(EXE)

test: $(EXE)
	set -e -o pipefail; \
	./$(EXE) \
	  | $(FILTER) > output.run; \
	cat output.txt | $(FILTER) | diff output.run -

.PHONY: run test

endif
