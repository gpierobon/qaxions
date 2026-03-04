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
# Optional Python bindings
# ------------------------------------------------------------------------------

WITH_PYTHON ?= 1

# ------------------------------------------------------------------------------
#  Platform detection
# ------------------------------------------------------------------------------

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    PLATFORM := mac
else
    PLATFORM := linux
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

PYTHON        := python3

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
    HDF5_INC := -I/usr/local/hdf5_serial/include
    HDF5_LIB := -L/usr/local/hdf5_serial/lib -lhdf5_cpp -lhdf5 -lsz -lz -ldl -lm
endif

# ------------------------------------------------------------------------------
#  FFTW
# ------------------------------------------------------------------------------

ifeq ($(PLATFORM),mac)
    FFTW_INC := -I$(BREW)/include
    FFTW_LIB := -L$(BREW)/lib -lfftw3_omp -lfftw3
else
    FFTW_INC := -I/usr/local/fftw3/include
    FFTW_LIB := -L/usr/local/fftw3/lib -lfftw3_omp -lfftw3
endif

# ------------------------------------------------------------------------------
#  Flags
# ------------------------------------------------------------------------------

CXXFLAGS := \
    $(STD) $(OPT) \
    $(OPENMP_CXX) $(OPENMP_INC) \
    $(HDF5_INC) $(FFTW_INC)

ifeq ($(WITH_PYTHON),1)
    CXXFLAGS += $(PYBIND_INCL)
endif

LIBS := \
    $(OPENMP_LIB) \
    $(HDF5_LIB) \
    $(FFTW_LIB)

# ------------------------------------------------------------------------------
#  Sources
# ------------------------------------------------------------------------------

SRC_CPP := $(shell find $(SRC_DIR) -type f \( -name '*.cpp' -o -name '*.cxx' \)\
	     ! -path '*/examples/*.cxx')

OBJS        := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SRC_CPP:.cpp=.o))
OBJS        := $(OBJS:.cxx=.o)

PYBIND_SRCS  := $(wildcard src/examples/*.cxx)
PYBIND_NAMES := $(notdir $(basename $(PYBIND_SRCS)))
PYBIND_OBJS  := $(patsubst src/%,build/%,$(PYBIND_SRCS:.cxx=.o))
PYBIND_MODS  := $(addprefix $(PY_PKG_DIR)/,$(addsuffix $(EXT_SUFFIX),$(PYBIND_NAMES)))

CORE_OBJS   := $(filter-out $(PYBIND_OBJS),$(OBJS))

# Create directories
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
	$(LINKER) $(OBJS) $(LIBS) -o $@

ifeq ($(PLATFORM),mac)
    PY_SHARED_FLAGS := -shared -undefined dynamic_lookup
    PY_RPATH := -Wl,-rpath,$(BREW)/lib
else
    PY_SHARED_FLAGS := -shared
    PY_RPATH := -Wl,-rpath,'$$ORIGIN'
endif

$(PY_PKG_DIR)/%$(EXT_SUFFIX): build/examples/%.o $(CORE_OBJS)
	$(LINKER) $(PY_SHARED_FLAGS) $(PY_RPATH) $^ $(LIBS) $(PY_LDFLAGS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cxx
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

$(BIN_DIR)/qaxi: $(SCRIPT_DIR)/qaxi
	mkdir -p $(BIN_DIR)
	cp $< $@
	chmod +x $@

clean:
	rm -rf $(BUILD_DIR) $(EXEC) $(PY_PKG_DIR)/*.so
	rm -rf $(BIN_DIR)/qaxi

run: $(EXEC)
	./$(EXEC)

info:
	@echo "Platform      = $(PLATFORM)"
	@echo "CXX           = $(CXX)"
	@echo "Python        = $(PYTHON)"
	@echo "WITH_PYTHON   = $(WITH_PYTHON)"
	@echo "EXT_SUFFIX    = $(EXT_SUFFIX)"
	@echo "CXXFLAGS      = $(CXXFLAGS)"
	@echo "LIBS          = $(LIBS)"

