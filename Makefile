# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

EXEC       := qaxions
PY_PKG     := pyqaxions
SRC_DIR    := src
BUILD_DIR  := build
PY_PKG_DIR := $(PY_PKG)
BIN_DIR    := bin
SCRIPT_DIR := scripts

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------

# Default to double precision
WITH_DOUBLE ?= 1
# Default with bindings, if pybind is found
WITH_PYTHON ?= 1
# For builds on HPC system Gadi (NCI)
WITH_GADI   ?= 0
# Enable GPU acceleration with 'make WITH_GPU=1'
WITH_GPU    ?= 0
# GPU architecture, sm_80 (A100), sm_90 (H200)
GPU_ARCH    ?= sm_60

# ------------------------------------------------------------------------------
#  Platform detection
# ------------------------------------------------------------------------------

UNAME_S := $(shell uname -s)


ifeq ($(UNAME_S),Darwin)
    PLATFORM := mac
else
    ifeq ($(WITH_GADI), 1)
	PLATFORM := gadi
    else
        PLATFORM := linux
    endif
endif

$(info Building for platform: $(PLATFORM))

# ------------------------------------------------------------------------------
#  Compilers
# ------------------------------------------------------------------------------

CXX    := c++
LINKER := $(CXX)

STD    := -std=c++17
OPT    := -O3 -g -Wall -Wextra -Wno-unknown-pragmas

# ------------------------------------------------------------------------------
#  Python tests / pybind11
# ------------------------------------------------------------------------------

PYTHON := python3

ifeq ($(WITH_GPU), 1)
	WITH_PYTHON = 0
        $(warning GPU (nvcc) build — disabling Python module build)
endif

ifeq ($(WITH_PYTHON),1)

    PYBIND_CHECK := $(shell $(PYTHON) -m pybind11 --includes 2>/dev/null)

    ifeq ($(PYBIND_CHECK),)
        $(warning pybind11 not found — disabling Python module build)
        WITH_PYTHON := 0
    else
        PYBIND_INCL := $(PYBIND_CHECK)
        PY_LDFLAGS  := $(shell $(PYTHON)-config --ldflags)
        EXT_SUFFIX  := $(shell $(PYTHON) -c "import sysconfig; \
                           print(sysconfig.get_config_var('EXT_SUFFIX'))")
    endif

endif

$(shell mkdir -p $(PY_PKG_DIR))
$(shell touch $(PY_PKG_DIR)/__init__.py)

# ------------------------------------------------------------------------------
#  OpenMP
# ------------------------------------------------------------------------------

ifeq ($(PLATFORM),mac)
    BREW       := $(shell brew --prefix)
    LIBOMP     := $(shell brew --prefix libomp)

    OPENMP_CXX := -Xclang -fopenmp
    OPENMP_INC := -I$(LIBOMP)/include
    OPENMP_LIB := -L$(LIBOMP)/lib -lomp
else
    OPENMP_CXX := -fopenmp
    OPENMP_INC :=
    OPENMP_LIB := -fopenmp
endif

# ------------------------------------------------------------------------------
#  HDF5
# ------------------------------------------------------------------------------

ifeq ($(PLATFORM),mac)
    HDF5_INC := -I$(BREW)/include
    HDF5_LIB := -L$(BREW)/lib -lhdf5_cpp -lhdf5 -lsz -lz -ldl -lm
else
    ifeq ($(WITH_GADI), 1)
    	HDF5_INC := -I/apps/hdf5/1.10.7/include
    	HDF5_LIB := -L/apps/hdf5/1.10.7/lib -lhdf5_cpp -lhdf5 -lsz -lz -lm
    else	
    	HDF5_INC := -I/usr/local/hdf5_serial/include
    	HDF5_LIB := -L/usr/local/hdf5_serial/lib -lhdf5_cpp -lhdf5 -lsz -lz -lm
    endif
endif

# ------------------------------------------------------------------------------
#  FFTW
# ------------------------------------------------------------------------------

ifeq ($(PLATFORM),mac)
    FFTW_INC := -I$(BREW)/include
    FFTW_LIB := -L$(BREW)/lib -lfftw3 -lfftw3f -lfftw3_omp -lfftw3f_omp
else
    ifeq ($(WITH_GADI), 1)
    	FFTW_INC := -I/apps/fftw3/3.3.8/include
    	FFTW_LIB := -L/apps/fftw3/3.3.8/lib -lfftw3 -lfftw3f
	FFTW_LIB += -lfftw3_omp_GNU -lfftw3f_omp_GNU
    else
	FFTW_INC := -I/usr/local/fftw3/include
    	FFTW_LIB := -L/usr/local/fftw3/lib -lfftw3 -lfftw3f 
	FFTW_LIB += -lfftw3_omp -lfftw3f_omp
    endif
endif


# ------------------------------------------------------------------------------
#  GPU / CUDA / cuFFT
# ------------------------------------------------------------------------------

ifeq ($(WITH_GPU),1)
    NVCC       := nvcc
    LINKER     := nvcc

    # Host-compiler flags forwarded through nvcc.
    # OpenMP must be passed via --compiler-options so nvcc hands it to the
    # host compiler rather than trying to interpret it itself.
    NVCC_HOST_FLAGS := $(OPENMP_CXX)

    NVCC_FLAGS := \
        -O3 -g \
        -arch=$(GPU_ARCH) \
        --expt-relaxed-constexpr \
        -std=c++17 \
        --compiler-options "$(NVCC_HOST_FLAGS)"

    # Expose GPU code paths to the host-side C++ translation units
    GPU_DEFINES := -DUSE_GPU

    GPU_LIBS := -lcudart -lcufft
endif

# ------------------------------------------------------------------------------
#  Flags
# ------------------------------------------------------------------------------

CXXFLAGS := \
    $(STD) $(OPT) \
    $(OPENMP_CXX) $(OPENMP_INC) \
    $(HDF5_INC) $(FFTW_INC)

ifeq ($(WITH_GPU),1)
    CXXFLAGS += $(GPU_DEFINES)
endif

ifeq ($(WITH_PYTHON),1)
    CXXFLAGS += $(PYBIND_INCL)
endif

ifeq ($(WITH_DOUBLE), 1)
    CXXFLAGS += -DUSE_DOUBLE
    NVCC_FLAGS += -DUSE_DOUBLE
endif

# Pure library flags — only -l/-L entries, no compiler driver flags.
# These are safe to pass to both g++ and nvcc directly.
LIBS := \
    $(HDF5_LIB) \
    $(FFTW_LIB)

ifeq ($(WITH_GPU),1)
    LIBS += $(GPU_LIBS)
endif

# Flags that contain compiler-driver options (-fopenmp, -Wl,... etc.) must be
# wrapped with -Xlinker when nvcc is the linker, otherwise nvcc rejects them.
ifeq ($(WITH_GPU),1)
  ifeq ($(PLATFORM),mac)
    LINK_FLAGS := -lomp
  else
    LINK_FLAGS := -lgomp
  endif
else
    LINK_FLAGS := $(OPENMP_LIB)
endif

# ------------------------------------------------------------------------------
#  Sources
# ------------------------------------------------------------------------------

SRC_CPP := $(shell find $(SRC_DIR) -type f \( -name '*.cpp' -o -name '*.cxx' \) \
             ! -path '*/examples/*.cxx')

ifeq ($(WITH_GPU),1)
    #SRC_CU := $(wildcard $(SRC_DIR)/*.cu)
    SRC_CU := $(shell find $(SRC_DIR) -type f -name '*.cu')
else
    SRC_CU :=
endif

# Derive object paths for C++/CXX sources
OBJS := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SRC_CPP:.cpp=.o))
OBJS := $(OBJS:.cxx=.o)

# Append CUDA object paths
OBJS += $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SRC_CU:.cu=.o))

PYBIND_SRCS  := $(wildcard src/examples/*.cxx)
PYBIND_NAMES := $(notdir $(basename $(PYBIND_SRCS)))
PYBIND_OBJS  := $(patsubst src/%,build/%,$(PYBIND_SRCS:.cxx=.o))
PYBIND_MODS  := $(addprefix $(PY_PKG_DIR)/,$(addsuffix $(EXT_SUFFIX),$(PYBIND_NAMES)))

CORE_OBJS := $(filter-out $(PYBIND_OBJS),$(OBJS))

# Create build subdirectories up front
$(shell mkdir -p $(sort $(dir $(OBJS) $(PYBIND_OBJS))) >/dev/null 2>&1)

# ------------------------------------------------------------------------------
#  Targets
# ------------------------------------------------------------------------------

.PHONY: all clean info run

ifeq ($(WITH_PYTHON),1)
    ALL_PY := $(PYBIND_MODS)
else
    ALL_PY :=
endif

all: $(EXEC) $(ALL_PY) $(BIN_DIR)/qaxi

$(EXEC): $(OBJS)
	$(LINKER) $(OBJS) $(LINK_FLAGS) $(LIBS) -o $@

# ------------------------------------------------------------------------------
#  Shared-library flags for Python extension modules
# ------------------------------------------------------------------------------

ifeq ($(PLATFORM),mac)
    PY_SHARED_FLAGS := -shared -undefined dynamic_lookup
    PY_RPATH_RAW    := -Wl,-rpath,$(BREW)/lib
else
    PY_SHARED_FLAGS := -shared
    PY_RPATH_RAW    := -Wl,-rpath,'$$ORIGIN'
endif

# Wrap rpath for nvcc the same way as LINK_FLAGS above
ifeq ($(WITH_GPU),1)
    PY_RPATH := $(addprefix -Xlinker ,$(PY_RPATH_RAW))
else
    PY_RPATH := $(PY_RPATH_RAW)
endif

$(PY_PKG_DIR)/%$(EXT_SUFFIX): build/examples/%.o $(CORE_OBJS)
	$(LINKER) $(PY_SHARED_FLAGS) $(PY_RPATH) $^ $(LINK_FLAGS) $(LIBS) $(PY_LDFLAGS) -o $@

# ------------------------------------------------------------------------------
#  Compilation rules
# ------------------------------------------------------------------------------

# C++ sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

# CXX sources (pybind11 examples and any .cxx translation units)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cxx
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

# CUDA sources — compiled unconditionally; only reachable when SRC_CU is non-empty
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cu
	$(NVCC) $(NVCC_FLAGS) $(GPU_DEFINES) $(HDF5_INC) $(FFTW_INC) -Xcompiler -fPIC -c $< -o $@

# ------------------------------------------------------------------------------
#  Helper script
# ------------------------------------------------------------------------------

$(BIN_DIR)/qaxi: $(SCRIPT_DIR)/qaxi
	mkdir -p $(BIN_DIR)
	cp $< $@
	chmod +x $@

# ------------------------------------------------------------------------------
#  Utility
# ------------------------------------------------------------------------------

clean:
	rm -rf $(BUILD_DIR) $(EXEC) $(PY_PKG_DIR)/*.so
	rm -rf $(BIN_DIR)/qaxi

run: $(EXEC)
	./$(EXEC)

info:
	@echo "Platform      = $(PLATFORM)"
	@echo "CXX           = $(CXX)"
	@echo "LINKER        = $(LINKER)"
	@echo "Python        = $(PYTHON)"
	@echo "WITH_DOUBLE   = $(WITH_DOUBLE)"
	@echo "WITH_PYTHON   = $(WITH_PYTHON)"
	@echo "WITH_GPU      = $(WITH_GPU)"
	@echo "GPU_ARCH      = $(GPU_ARCH)"
	@echo "EXT_SUFFIX    = $(EXT_SUFFIX)"
	@echo "CXXFLAGS      = $(CXXFLAGS)"
	@echo "NVCC_FLAGS    = $(NVCC_FLAGS)"
	@echo "LIBS          = $(LIBS)"
	@echo "LINK_FLAGS    = $(LINK_FLAGS)"
