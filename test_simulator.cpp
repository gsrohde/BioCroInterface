// Compile with the flag -DVERBOSE=true to get verbose output.
#ifndef VERBOSE
#define VERBOSE false
#endif

#include <gtest/gtest.h>

#include "BioCro.h"
#include "print_result.h"

using Module_factory = BioCro::Standard_BioCro_library_module_factory;

/*
 * Here we test basic usage of a Simulator object, showing how to
 * construct and run such objects.  First we show how construction
 * works using named arguments of the requisite types for the
 * system-related parameters.  Then we show we can supply arguments
 * directly as initializer lists.
 *
 * The form of the tests is slightly unusual: we call a function that
 * exits with code 0 if no problems were encountered, and the tests
 * test that the function did indeed exit with code 0.
 */

BioCro::Simulator get_simulation() {

    BioCro::State initial_state { {"position", 0}, {"velocity", 1} };
    BioCro::Parameter_set parameters
        { {"mass", 10}, {"spring_constant", 0.1}, {"timestep", 1} };
    BioCro::System_drivers drivers
        { {"time",  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }} };
    BioCro::Module_set steady_state_modules {};
    BioCro::Module_set derivative_modules {
        Module_factory::retrieve("harmonic_oscillator")
    };

    return BioCro::Simulator {
        initial_state,
        parameters,
        drivers,
        steady_state_modules,
        derivative_modules,
        "homemade_euler",
        1,
        0.0001,
        0.0001,
        200
    };
}

class BioCroSimulationTest : public ::testing::Test {
   protected:
    BioCroSimulationTest() :bs{get_simulation()} {
    }
    BioCro::Simulator bs;

    void trial_simulation() {
        BioCro::Simulation_result result = bs.run_simulation();
        if (VERBOSE) print_result(result);

        // If we get here, constructing the simulator and running the
        // simulation proceeded normally.
        exit(0);
    }
};

TEST_F(BioCroSimulationTest, CorrectSimulation) {

    ASSERT_EXIT(trial_simulation(),
                ::testing::ExitedWithCode(0),
                ".*");
}

// This test shows that we don't need to declare variables with types
// State, Parameter_set, System_drivers, and Module_set in order to
// construct a Simulator.  Instead, we can supply the arguments
// directly as initializer lists.
TEST_F(BioCroSimulationTest, SimulatorConstructedFromInitializerLists) {
    bs = BioCro::Simulator(

        // system-related arguments
        { {"position", 0}, {"velocity", 1} },
        { {"mass", 10}, {"spring_constant", 0.1}, {"timestep", 1} },
        { {"time",  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }} },
        {},
        {Module_factory::retrieve("harmonic_oscillator")},

        // solver-related arguments
        "homemade_euler",
        1,
        0.0001,
        0.0001,
        200

    );
    
    ASSERT_EXIT(trial_simulation(),
                ::testing::ExitedWithCode(0),
                ".*");
}
