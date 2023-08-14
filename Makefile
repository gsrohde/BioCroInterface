# This extension works for macOS
LIBRARY_FILE_EXTENSION = so

BIOCRO_LIB = BioCro.$(LIBRARY_FILE_EXTENSION)
EXTERNAL_BIOCRO_LIB = testBML.$(LIBRARY_FILE_EXTENSION)

# root directories for BioCro and testBML header files
BIOCRO_SOURCE_PATH = ../src
EXTERNAL_BIOCRO_LIB_SOURCE_PATH = testBML/src

# Boost files
BOOST_PATH = ../inc

BIOCRO_INCLUDES = -I $(BIOCRO_SOURCE_PATH) -I $(BOOST_PATH)

SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
EXE = $(OBJECTS:.o=)
RUN_TARGETS = $(patsubst %,run_%,$(EXE))

# Override with "make <target> VERBOSE=true"
VERBOSE = false

.PHONY: clean $(RUN_TARGETS)

run_all_tests: test_all
	./test_all

# Convenient target aliases
0: run_all_tests
1: run_segfault_test
2: run_test_dynamical_system
3: run_test_harmonic_oscillator
4: run_test_module_creator
5: run_test_module_evaluation
6: run_test_module_factory_functions
7: run_test_module_object
8: run_test_multiple_module_libraries
9: run_test_repeat_runs
10: run_test_simulator

$(RUN_TARGETS) : run_% : %
	./$<

$(BIOCRO_LIB): $(BIOCRO_SOURCE_PATH)/$(BIOCRO_LIB)
	cp $< $@

$(EXTERNAL_BIOCRO_LIB): $(EXTERNAL_BIOCRO_LIB_SOURCE_PATH)/$(EXTERNAL_BIOCRO_LIB)
	cp $< $@

$(BIOCRO_SOURCE_PATH)/$(BIOCRO_LIB):
	echo "Build BioCro before running these tests."


test_all : $(OBJECTS) $(EXTERNAL_BIOCRO_LIB) $(BIOCRO_LIB)
	clang++ -std=c++14 -o $@ $(BIOCRO_LIB) $^ -lgtest_main -lgtest

$(EXE) : % : %.o $(BIOCRO_LIB)
	clang++ -std=c++14 -o $@ $^ -lgtest_main -lgtest

# extra prerequisite for test_module_evaluation and test_harmonic_oscillator
test_module_evaluation test_harmonic_oscillator: Random.o

# extra prerequisite for test_multiple_module_libraries
test_multiple_module_libraries: $(EXTERNAL_BIOCRO_LIB)



# header file dependencies
test_simulator.o test_dynamical_system.o test_harmonic_oscillator.o \
    test_repeat_runs.o: print_result.h
test_harmonic_oscillator.o test_repeat_runs.o test_module_evaluation.o \
    test_module_factory_functions.o test_module_creator.o \
    test_module_object.o: BioCro.h
test_dynamical_system.o test_simulator.o test_multiple_module_libraries.o: \
    BioCro_Extended.h
test_module_evaluation.o test_harmonic_oscillator.o Random.o: Random.h
test_repeat_runs.o: safe_simulators.h

segfault_test : Random.o


$(OBJECTS) : %.o : %.cpp
	clang++ -std=c++14 $(BIOCRO_INCLUDES) $< -o $@ -c -DVERBOSE=$(VERBOSE)

clean:
	rm -f $(EXE) $(OBJECTS)
