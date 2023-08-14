#ifndef VERBOSE
#define VERBOSE false
#endif


#include <gtest/gtest.h>

#include <iostream>

#include "BioCro.h"

#include "Random.h"

using std::cout;
using std::endl;

using math_constants::pi;
using Module_factory = BioCro::Standard_BioCro_library_module_factory;

class ModuleEvaluationTest : public ::testing::Test {
 protected:
    Rand_double double_gen { -100, 100 };
    Rand_double pos_double_gen { 1e-5, 100 };

    BioCro::Variable_settings outputs;

    void print_quantities(BioCro::Variable_settings quantities) {
        for (BioCro::Variable_setting& item : quantities) {
            cout << item.first << ": " << item.second << endl;
        }
    }

};

TEST_F(ModuleEvaluationTest, DifferentialModule) {

    BioCro::Module_creator w = Module_factory::retrieve("harmonic_oscillator");
                            
    const BioCro::Variable_settings inputs {
        {"position", double_gen()},
        {"velocity", double_gen()},
        // The mass and spring constant must be positive:
        {"mass", pos_double_gen()},
        {"spring_constant", pos_double_gen()}
    };

    // Get the module's outputs and add them to the output list with default
    // values of 0.0. Since derivative modules add their output values to
    // the values in outputs, the result only makes sense if each
    // parameter is initialized to 0.
    for (string& param : w->get_outputs()) {
        outputs[param] = 0.0;
    }

    BioCro::Module module = w->create_module(inputs, &outputs);

    module->run();

    if (VERBOSE) {
        print_quantities(inputs);
        print_quantities(outputs);
    }

    // dx/dt = v    
    EXPECT_DOUBLE_EQ(outputs.at("position"), inputs.at("velocity"));
    // dv/dt = a = -kx/m
    EXPECT_DOUBLE_EQ(outputs.at("velocity"),
                     -inputs.at("spring_constant") * inputs.at("position") / inputs.at("mass"));
}

TEST_F(ModuleEvaluationTest, DirectModule) {

    BioCro::Module_creator w = Module_factory::retrieve("solar_position_michalsky");
                            
    // Use values for Urbana, Illinois (40.0932N 88.20175W) at 5:48 CDT on July 19, 2023,
    // the time predicted as the sunrise time on timeanddate.com:
    const BioCro::Variable_settings inputs {
        {"lat", 40.0932},
        {"longitude", -88.20175},
        {"time", 200 + (5.0 + 48.0/60)/24},
        {"time_zone_offset", -5},
        {"year", 2023},
    };

    // Get the module's outputs and add them to the output list with default
    // values of 0.0.
    for (string& param : w->get_outputs()) {
        outputs[param] = 0.0;
    }

    BioCro::Module module = w->create_module(inputs, &outputs);

    module->run();

    if (VERBOSE) {
        print_quantities(inputs);
        print_quantities(outputs);
    }

    EXPECT_NEAR(outputs["cosine_zenith_angle"], 0, 1.1e-2);

    // For a more meaningful comparison, get the zenith angle itself
    // and check that it is close to 90 degrees.
    double zenith_angle_in_degrees{acos(outputs["cosine_zenith_angle"]) * 180/pi};

    // This seems to be a higher tolerance value than we would expect
    // to have to use.
    EXPECT_NEAR(zenith_angle_in_degrees, 90, 0.621);
}

// This test is disabled because it is designed to fail.  It shows
// what happens when we create a module using a non-persistent
// parameter value for the first parameter (i.e., the input_quantities
// parameter) of create_module.  To run it (and only it) anyway, use
// the command
//
//     ./test_module_evaluation --gtest_also_run_disabled_tests --gtest_filter="*.DISABLED*"
//
// See the file test_module_object.cpp for more.
TEST_F(ModuleEvaluationTest, DISABLED_IncorrectlyConstructedDifferentialModule) {

    BioCro::Module_creator w = Module_factory::retrieve("harmonic_oscillator");

    // Get the module's outputs and add them to the output list with default
    // values of 0.0. Since derivative modules add their output values to
    // the values in outputs, the result only makes sense if each
    // parameter is initialized to 0.
    for (string& param : w->get_outputs()) {
        outputs[param] = 0.0;
    }
    const BioCro::Variable_settings inputs {
        {"position", 9},        // x
        {"velocity", -12},      // v
        // The mass and spring constant must be positive:
        {"mass", 50},           // m
        {"spring_constant", 30} // k
    };
    BioCro::Module good_module = w->create_module(inputs, &outputs);
    
    good_module->run();

    if (VERBOSE) {
        print_quantities(inputs);
        print_quantities(outputs);
    }

    // dx/dt = v    
    EXPECT_DOUBLE_EQ(outputs["position"], -12);
    // dv/dt = a = -kx/m
    EXPECT_DOUBLE_EQ(outputs["velocity"],
                     -30.0 * 9 / 50);

    // This module is "bad" we cause we create it using a
    // non-persistent input_quantities parameter value.
    BioCro::Module bad_module = w->create_module({
        {"position", 19},       // x
        {"velocity", -12},      // v
        // The mass and spring constant must be positive:
        {"mass", 50},           // m
        {"spring_constant", 40} // k
    }, &outputs);

    // reset outputs to 0
    for (string& param : w->get_outputs()) {
        outputs[param] = 0.0;
    }

    bad_module->run();

    if (VERBOSE) {
        print_quantities(outputs);
    }

    // dx/dt = v    
    EXPECT_DOUBLE_EQ(outputs["position"], -112);
    // dv/dt = a = -kx/m
    EXPECT_DOUBLE_EQ(outputs["velocity"],
                     -30.0 * 9 / 50);

    
}
