// Here we test the use of multiple module libraries at once.
//
// We show that we can use two modules from different module libraries
// that have the same name as long as they are otherwise compatible.
// In MultipleModuleLibrariesTest.CompatibleModules, we show this to
// be the case with two slightly different differential modules, both
// called "thermal_time_linear".  (As noted below, CompatibleModules
// is a slight misnomer for this test.  Although the modules used are
// formally compatible, since they make different assumptions about
// the units for timestep, their use together is nonsensical.)
//
// On the other hand, identical direct modules from different
// libraries will conflict since their outputs overlap, as we show in
// MultipleModuleLibrariesTest.ConflictingModules.  (Note that we
// could use two identically-named direct modules from different
// libraries if their output quantity sets were disjoint.)

// Compile with the flag -DVERBOSE=true to get verbose output.
#ifndef VERBOSE
#define VERBOSE false
#endif

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "BioCro_Extended.h"
#include "print_result.h"

using ::testing::MatchesRegex;

using Module_factory = BioCro::Standard_BioCro_library_module_factory;
using Module_factory_2 = BioCro::Test_BioCro_library_module_factory;

class MultipleModuleLibrariesTest : public ::testing::Test {
   protected:
    MultipleModuleLibrariesTest() :bs{get_simulator()} {
    }

    void trial_simulation() {
        bs = get_simulator();
        result = bs.run_simulation();
        if (VERBOSE) print_result(result);
    }

    // These are defaults.  Individual tests can alter them.
    BioCro::State initial_state { {"TTc", 0} };
    BioCro::Parameter_set parameters
        { {"timestep", 1} };
    BioCro::System_drivers drivers
        { {"time", { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 } },
          {"temp", { 5, 8, 10, 15, 20, 20, 25, 30, 32, 40} } };
    BioCro::Module_set direct_modules {};
    BioCro::Module_set differential_modules {};

    BioCro::Simulation_result result;

private:
    BioCro::Simulator get_simulator();

    // This must be declared last because its initialization depends
    // on the parameters for its construction being initialized first:
    BioCro::Simulator bs;
};

BioCro::Simulator MultipleModuleLibrariesTest::get_simulator() {
    return BioCro::Simulator {
        initial_state,
        parameters,
        drivers,
        direct_modules,
        differential_modules,
        "homemade_euler",
        1,
        0.0001,
        0.0001,
        200
    };
}

TEST_F(MultipleModuleLibrariesTest, CompatibleModules) {
    parameters.insert({{"sowing_time", 0}, {"tbase", 10} });
    std::initializer_list<BioCro::Module_creator> additional_modules {
        Module_factory::retrieve("thermal_time_linear"),
        Module_factory_2::retrieve("thermal_time_linear")
    };
    differential_modules.insert(differential_modules.end(), Module_factory::retrieve("thermal_time_linear"));
    trial_simulation();
    vector<double> TTc_values = result["TTc"];
    size_t highest_index = TTc_values.size() - 1;
    double final_TTc_value = TTc_values[highest_index];
    constexpr double expected_value {3 + 5.0/12};
    ASSERT_DOUBLE_EQ(final_TTc_value, expected_value);

    // The thermal_time_linear module in the testBML module assumes
    // timestep values in days rather than hours, so the resulting TTc
    // value (the change per timestep) is 24 times as large.  So when
    // we add this module into the set of differential modules, since
    // differential modules are additive, we get a final TTc value 25
    // times as large as before.
    //
    // Note that when we make our own module libraries, we can assume
    // any units we want for the timestep.  But of course it makes no
    // realistic sense to mix modules having different assumptions
    // about the timestep units, and we do so here only for the sake
    // of demonstration.
    differential_modules.insert(differential_modules.end(), Module_factory_2::retrieve("thermal_time_linear"));
    trial_simulation();
    TTc_values = result["TTc"];
    final_TTc_value = TTc_values[highest_index];
    ASSERT_DOUBLE_EQ(final_TTc_value, expected_value * 25);
        
}

// Show that direct modules having outputs in common conflict, even if
// they are from different module libraries.
TEST_F(MultipleModuleLibrariesTest, ConflictingModules) {
    parameters.insert({{"lat", 44}, {"longitude", -121},
                       {"time_zone_offset", -8}, {"year", 2023} });

    direct_modules = {Module_factory::retrieve("solar_position_michalsky"),
                      Module_factory_2::retrieve("solar_position_michalsky")};

    EXPECT_THROW(trial_simulation(), std::logic_error);

    // The solar_position_michalsky module is defined identically in
    // standardBML and testBML (except for the namespace, of course).
    // In particular, they the same set of outputs, resulting in a
    // "quantities defined more than once" error.
    std::string message_match_string = "Thrown by dynamical_system::"
        "dynamical_system: the supplied inputs cannot form a valid dynamical "
        "system.*"
        "The following quantities were defined more than once in the inputs:.*";

    // Test that the expected exception is thrown.
    EXPECT_THROW({
        try
        {
            trial_simulation();
        }
        catch( const std::logic_error& e )
        {
            // Test that the exeception has the correct message.
            EXPECT_THAT(e.what(), MatchesRegex(message_match_string));
            throw;
        }
    }, std::logic_error);

}
