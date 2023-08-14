// Compile with the flag -DVERBOSE=true to get verbose output.
#ifndef VERBOSE
#define VERBOSE true
#endif

#include <gtest/gtest.h>

#include "safe_simulators.h"
#include "BioCro.h"
#include "print_result.h"

using Module_factory = BioCro::Standard_BioCro_library_module_factory;

class BiocroSimulationTest : public ::testing::Test {
   protected:
    BioCro::State initial_state { {"TTc", 0} };
    BioCro::Parameter_set parameters { {"sowing_time", 0},
                                       {"tbase", 5},
                                       {"temp", 11},
                                       {"timestep", 1} };
    BioCro::System_drivers drivers { {"time",  { 0, 1, 2, 3, 4, 5 }} };
    BioCro::Module_set steady_state_modules;
    BioCro::Module_set derivative_modules
        { Module_factory::retrieve("thermal_time_linear") };
};

// "run_simulation()" should be idempotent.  Alternatively, an
// exception should be thrown if the user attempts to run it more than
// once.
//
// This test checks that each time the simulation is run, the result
// is the same.  Currently this fails unless there are no differential
// modules.  This is because the state of the differential quantities
// is not reset at the completion of a run so that on subsequent runs,
// the initial state of these quantities will not correspond to the
// initial state given to the BioCro::Simulator constructor.

TEST_F(BiocroSimulationTest, DISABLED_runSimulationIsIdempotent) {
    BioCro::Simulator sim {
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

    const BioCro::Simulation_result first_result = sim.run_simulation();
    const BioCro::Simulation_result second_result = sim.run_simulation();

    if (VERBOSE) print_result(first_result);
    if (VERBOSE) print_result(second_result);

    for (auto& item : first_result) {
        string quantity_name {item.first};
        size_t duration {item.second.size()};
        for (size_t i {0}; i < duration; ++i) {
            EXPECT_DOUBLE_EQ(first_result.at(quantity_name)[i],
                             second_result.at(quantity_name)[i]);
        }
    }
}

TEST_F(BiocroSimulationTest, runSimulationIsIdempotent) {
    BioCro::Idempotent_simulator idem_sim {
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

    const BioCro::Simulation_result first_result = idem_sim.run_simulation();
    const BioCro::Simulation_result second_result = idem_sim.run_simulation();

    if (VERBOSE) print_result(first_result);
    if (VERBOSE) print_result(second_result);

    for (auto& item : first_result) {
        string quantity_name {item.first};
        size_t duration {item.second.size()};
        for (size_t i {0}; i < duration; ++i) {
            EXPECT_DOUBLE_EQ(first_result.at(quantity_name)[i],
                             second_result.at(quantity_name)[i]);
        }
    }


    BioCro::Alternate_idempotent_simulator alt_sim {
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

    const BioCro::Simulation_result first_alt_result = alt_sim.run_simulation();
    const BioCro::Simulation_result second_alt_result = alt_sim.run_simulation();

    if (VERBOSE) print_result(first_alt_result);
    if (VERBOSE) print_result(second_alt_result);

    for (auto& item : first_alt_result) {
        string quantity_name {item.first};
        size_t duration {item.second.size()};
        for (size_t i {0}; i < duration; ++i) {
            EXPECT_DOUBLE_EQ(first_alt_result.at(quantity_name)[i],
                             second_alt_result.at(quantity_name)[i]);
        }
    }
}

TEST_F(BiocroSimulationTest, cannotRunSingleUseSimulatorTwice) {
    BioCro::Single_use_simulator single_use_sim {
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
    const BioCro::Simulation_result first_result = single_use_sim.run_simulation();

    EXPECT_THROW({
            single_use_sim.run_simulation();
        }, std::runtime_error);
}
