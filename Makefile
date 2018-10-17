CUDA_MAJOR_NUM ?= 5
CUDA_MINOR_NUM ?= 2
CARLSIM_CUDAVER ?= 7

CARLSIM_LIB_DIR ?= /home/gdetorak/CARL/
CUDA_INSTALL_PATH ?= /opt/cuda

# OS name (Linux or Darwin) and architecture (32 bit or 64 bit).
OS_SIZE 	:=$(shell uname -m | sed -e "s/i.86/32/" -e "s/x86_64/64/")
OS_LOWER 	:=$(shell uname -s 2>/dev/null | tr [:upper:] [:lower:])
OS_UPPER 	:=$(shell uname -s 2>/dev/null | tr [:lower:] [:upper:])
DARWIN  	:=$(strip $(findstring DARWIN, $(OS_UPPER)))

# variable defintions
CXX = g++
CC  = g++
NVCC = $(CUDA_INSTALL_PATH)/bin/nvcc
NVCFLAGS = -c --compiler-options "-std=c++0x" --compiler-options "-fPIC"
CARLSIM_FLAGS = -use_fast_math -O2 --compiler-options '-std=c++0x'

IDIR = ./include/
OBJ = ./obj
LIB_ = ./lib/libnsat.so

# if Mac OS X, include these flags
ifeq ($(DARWIN),DARWIN)
	CARLSIM_FLAGS +=-Xlinker -lstdc++ -lc++
endif

ifeq ($(strip $(CPU_ONLY)),1)
	CARLSIM_FLAGS += -D__CPU_ONLY__
	NVCC = $(CXX)
else
	# add compute capability to compile flags
	CARLSIM_FLAGS += -arch sm_$(CUDA_MAJOR_NUM)$(CUDA_MINOR_NUM)
	ifeq (${strip ${CUDA_MAJOR_NUM}},1)
		CARLSIM_FLAGS += -D__NO_ATOMIC_ADD__
	endif

	# add CUDA version to compile flags
	CARLSIM_FLAGS += -D__CUDA$(CARLSIM_CUDAVER)__

	# load appropriate CUDA flags
	ifneq (,$(filter $(CARLSIM_CUDAVER),3 4))
		CARLSIM_INCLUDES = -I${NVIDIA_SDK}/C/common/inc/
		CARLSIM_LFLAGS = -L${NVIDIA_SDK}/C/lib
		CARLSIM_LIBS = -lcutil_x86_64
	else
		CARLSIM_INCLUDES = -I$(CUDA_INSTALL_PATH)/samples/common/inc/
		CARLSIM_LFLAGS =
		CARLSIM_LIBS =
	endif
endif

project := test_nsat
output := *.dot *.dat *.log *.csv

local_src  := src/main_$(project).cpp
local_prog := bin/$(project)
local_objs := src/nsat_core.cpp src/connx_core.cpp src/auxiliary.cpp
unity_objs := src/unity.cpp

CARLSIM_FLAGS += -I$(CARLSIM_LIB_DIR)/include/kernel \
				 -I$(CARLSIM_LIB_DIR)/include/interface \
				 -I$(CARLSIM_LIB_DIR)/include/spike_monitor \
				 -I$(CARLSIM_LIB_DIR)/include/connection_monitor \
				 -I$(CARLSIM_LIB_DIR)/include/spike_generators \
				 -I$(CARLSIM_LIB_DIR)/include/visual_stimulus \
				 -I$(CARLSIM_LIB_DIR)/include/simple_weight_tuner \
				 -I$(CARLSIM_LIB_DIR)/include/stopwatch \
				 -I$(CARLSIM_LIB_DIR)/include/group_monitor
CARLSIM_FLAGS += -I$(IDIR)
CARLSIM_LIBS  += -L$(CARLSIM_LIB_DIR)/lib -lCARLsim

output_files += $(local_prog)

.PHONY: clean distclean devtest

test_nsat: $(local_src) $(local_objs)
	$(NVCC) $(CARLSIM_INCLUDES) $(CARLSIM_FLAGS) $(local_src) $(local_objs) -o ./bin/$@ $(CARLSIM_LFLAGS) $(CARLSIM_LIBS)

obj_test_nsat: $(local_objs)
	$(NVCC) $(NVCFLAGS) $(CARLSIM_INCLUDES) $(CARLSIM_FLAGS) $(local_objs) $(CARLSIM_LFLAGS) $(CARLSIM_LIBS)

lib_test_nsat: nsat_core.o connx_core.o
	$(NVCC) $(NVCFLAGS) $(CARLSIM_INCLUDES) $(CARLSIM_FLAGS) $(unity_objs) $(CARLSIM_LFLAGS) $(CARLSIM_LIBS)
	g++ -shared -o $(LIB_) unity.o $(CARLSIM_LIBS) -L/opt/cuda/lib64 -lcudart

clean:
	rm -f $(output_files) *.o *.so

distclean:
	rm -f $(output_files) results/*

devtest:
	@echo $(CARLSIM_FLAGS)
