# CPP Flags
CPP_FLAGS=-Werror -Wall -std=c++14 -x c++ -fmax-errors=1

# Sources directories
SRC_LIB_DIR=src/lib
SRC_LIB_INCLUDE_DIR=$(SRC_LIB_DIR)/include
SRC_GTEST_DIR=src/googletest
SRC_GTEST_INCLUDE_DIR=$(SRC_GTEST_DIR)/include
SRC_RUNNER_DIR=src/runner
SRC_TESTER_DIR=src/tester

# Some src files lists
LIB_INCLUDE_FILES=$(shell find -L "$(SRC_LIB_INCLUDE_DIR)/integral_image" -type f -name '*.hpp' )
GTEST_SRC_FILES=$(shell find -L "$(SRC_GTEST_DIR)" -type f \( -name '*.cc' -o -name '*.h' \) )
GTEST_INCLUDE_FILES=$(shell find -L "$(SRC_GTEST_INCLUDE_DIR)/gtest" -type f -name '*.h' )

# Binary information
BIN_DIR=bin
LIB_NAME=IntegralImgComputer
LIB_FILENAME=lib$(LIB_NAME).so
GTEST_LIB_NAME=gtest
GTEST_LIB_FILENAME=lib$(GTEST_LIB_NAME).a
RUNNER_FILENAME=runner.out
TESTER_FILENAME=tester.out

# OpenCV
OPENCV_INCLUDE_DIR=/usr/local/include
OPENCV_DIR=/usr/local/lib
OPENCV_IMGCODECS_LIB_NAME=opencv_imgcodecs

# OpenCV include files list
OPENCV_INCLUDE_FILES=$(shell find -L "$(OPENCV_INCLUDE_DIR)" -type f -name '*.hpp')

# Phony targets

.PHONY: all lib gtest runner tester clean_o clean 

all: runner tester

lib: $(BIN_DIR)/$(LIB_FILENAME)

gtest: $(BIN_DIR)/$(GTEST_LIB_FILENAME)

runner: $(BIN_DIR)/$(RUNNER_FILENAME)

tester: $(BIN_DIR)/$(TESTER_FILENAME)

# Library
$(BIN_DIR)/IntegralImgComputer.o: $(SRC_LIB_DIR)/IntegralImgComputer.cpp           \
                                  $(LIB_INCLUDE_FILES)                             \
                                  $(OPENCV_INCLUDE_FILES)
	g++ $(CPP_FLAGS) -isystem "$(OPENCV_INCLUDE_DIR)" -fPIC -c $< -o $@

$(BIN_DIR)/$(LIB_FILENAME): $(BIN_DIR)/IntegralImgComputer.o
	g++ -shared -Wl,-soname,$(LIB_FILENAME) $<                                       \
  -l$(OPENCV_IMGCODECS_LIB_NAME)                                                   \
  -o $@

# Runner
$(BIN_DIR)/runner.o: $(SRC_RUNNER_DIR)/runner.cpp                                  \
                     $(SRC_RUNNER_DIR)/aux.hpp
	g++ $(CPP_FLAGS) -c $< -o $@

$(BIN_DIR)/runner_aux.o: $(SRC_RUNNER_DIR)/aux.cpp                                 \
                         $(SRC_RUNNER_DIR)/aux.hpp                                 \
                         $(LIB_INCLUDE_FILES)
	g++ -isystem "$(SRC_LIB_INCLUDE_DIR)" $(CPP_FLAGS) -c $< -o $@

$(BIN_DIR)/$(RUNNER_FILENAME):   $(BIN_DIR)/runner.o                               \
                                 $(BIN_DIR)/runner_aux.o                           \
                               | $(BIN_DIR)/$(LIB_FILENAME)
	g++ -Wl,-rpath=. $^ $| -pthread -o $@

# Google test library
$(BIN_DIR)/gtest-all.o: $(GTEST_SRC_FILES)
	g++ $(CPP_FLAGS) -isystem $(SRC_GTEST_INCLUDE_DIR) -I$(SRC_GTEST_DIR)            \
  -c $(SRC_GTEST_DIR)/src/gtest-all.cc                                             \
  -o $@

$(BIN_DIR)/$(GTEST_LIB_FILENAME): $(BIN_DIR)/gtest-all.o
	ar -rv $@ $<

# Tester
$(BIN_DIR)/tester.o: $(SRC_TESTER_DIR)/tester.cpp                                  \
                     $(GTEST_INCLUDE_FILES)                                        \
                     $(LIB_INCLUDE_FILES)
	g++ $(CPP_FLAGS)                                                                 \
  -isystem "$(SRC_GTEST_INCLUDE_DIR)" -isystem "$(SRC_LIB_INCLUDE_DIR)"            \
  -c $< -o $@

$(BIN_DIR)/$(TESTER_FILENAME):   $(BIN_DIR)/tester.o                               \
                                 $(BIN_DIR)/$(GTEST_LIB_FILENAME)                  \
                               | $(BIN_DIR)/$(LIB_FILENAME)
	g++ $^ $| -pthread -Wl,-rpath=. -o $@


# Cleaning

clean_o:
	rm -vf bin/*.o bin/*.a

clean:
	rm -vfr bin/*

