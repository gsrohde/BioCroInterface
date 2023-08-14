// Here we show that BioCro modules may be created and used directly,
// as opposed to being obtained using the module_creator class, but
// that we have to be careful when doing so.  (It should, though, be
// noted that even when a module is obtained using module_creator,
// care must be exercised.)  In particular, in order to obtain the
// expected output, the input quantities object passed to the module
// constructor must persist at least until the module is run.  This
// means, for example, that we can't pass the input quantities as a
// state_map literal, even though it may seem natural to do so when we
// simply want to use a module outside of the context of a simulation.
//
// Note that in many of the tests designed to demonstrate the pitfalls
// of using a module in the wrong way, the module occassionally
// returns the result one would expect if the module were used
// correctly, and thus the test fails because the module "worked".
// (Such tests, that fail only intermittently, are called "flakey
// tests".  See
// https://testing.googleblog.com/2016/05/flaky-tests-at-google-and-how-we.html
// and
// https://mir.cs.illinois.edu/marinov/publications/LuoETAL14FlakyTestsAnalysis.pdf.)
// Since these tests are only there to demonstrate how *not* to use a
// module object, we aren't especially concerned that they sometimes
// fail.  (In this sense, they are fundamentally different from the
// flakey tests discussed in the two cited articles.  We would remove
// or disable these tests in a test battery being used for CI so as
// not to be distracted by failing tests that really don't pertain to
// code integrity.)
//
// The flakey tests include AlteredReferencedObject, OutputNotDoubled,
// RvalueInputNotOK, and LiteralInputNotOK.  Failures are usually rare
// but consistently occur if the test is run often enough.  For
// example, when the command
//
//     ./test_module_object --gtest_filter="*.LiteralInputNotOK" --gtest_repeat=60000 | grep "FAILED.*ModuleObjectTest.*[^)]$" | wc -l
//
// is used to repeat the LiteralInputNotOK test 60,000 times, the
// failure rate in those 60,000 repetitions is often in the single
// digits.  But occassionally, it may fail many more times (12,889 in
// one count!) or not at all.
//
// Note that because by working with a module class directly we are
// working with BioCro at relatively low level, we include BioCro
// header files directly (in this case, thermal_time_linear.h, which
// itself includes state_map.h) rather than using the interface
// provided by BioCro.h.  (Compare with test_module_evaluation.cpp.)

#include <gtest/gtest.h>
#include <gmock/gmock.h> // for matchers Not and DoubleEq

#include <iostream>

// The commented-out include is included in thermal_time_linear.h.
#include <module_library/thermal_time_linear.h> // for standardBML::thermal_time_linear
// #include <framework/state_map.h> // for state_map

using testing::Not;
using testing::DoubleEq;

class ModuleObjectTest : public ::testing::Test {
   protected:
    const double input_time {200};
    const double input_sowing_time {100};
    const double input_temp {25};
    const double input_tbase {1};

    state_map input_quantities {{"time", input_time},
                                {"sowing_time", input_sowing_time},
                                {"temp", input_temp},
                                {"tbase", input_tbase}};

    state_map output {{"TTc", 0}};

    double expected_output_value = input_time < input_sowing_time ? 0.0
                                 : input_temp <= input_tbase      ? 0.0
                                 :                                  (input_temp - input_tbase)/24.0;
};

TEST_F(ModuleObjectTest, CorrectDirectUsage)
{
    standardBML::thermal_time_linear ttl {input_quantities, &output};
    ttl.run();

    EXPECT_DOUBLE_EQ(output.at("TTc"), expected_output_value);
}

TEST_F(ModuleObjectTest, AlteredReferencedObject)
{
    standardBML::thermal_time_linear ttl {input_quantities, &output};
    input_quantities = {};
    ttl.run();

    // We don't get the expected value because the module references
    // the external input_quantites object, which has changed by the
    // time we run the module.
    EXPECT_THAT(output.at("TTc"), Not(DoubleEq(expected_output_value)));
}

TEST_F(ModuleObjectTest, AlterationAfterRunOK)
{
    standardBML::thermal_time_linear ttl {input_quantities, &output};
    ttl.run();
    input_quantities = {};

    EXPECT_DOUBLE_EQ(output.at("TTc"), expected_output_value);
}

TEST_F(ModuleObjectTest, RerunningDoublesTheOutput)
{
    standardBML::thermal_time_linear ttl {input_quantities, &output};
    ttl.run();
    ttl.run();

    // This, more precisely, doubles the increase; but since we
    // started from zero, this is the same as doubling the output
    // value.
    EXPECT_DOUBLE_EQ(output.at("TTc"), 2 * expected_output_value);
}

TEST_F(ModuleObjectTest, OutputNotDoubled)
{
    standardBML::thermal_time_linear ttl {input_quantities, &output};
    ttl.run();
    input_quantities = {};
    ttl.run();

    // When we reset the input_quantities before running the module a
    // second time, the result will no longer be double what we got
    // from running it once.
    EXPECT_THAT(output.at("TTc"), Not(DoubleEq(2 * expected_output_value)));
}

TEST_F(ModuleObjectTest, RvalueInputNotOK)
{
    standardBML::thermal_time_linear ttl {
                                          {{"time", input_time},
                                           {"sowing_time", input_sowing_time},
                                           {"temp", input_temp},
                                           {"tbase", input_tbase}},
                                          &output};
    ttl.run();

    // We don't get the expected value because the module references
    // the ephemeral first argument to the module construtor.
    EXPECT_THAT(output.at("TTc"), Not(DoubleEq(expected_output_value)));
}

TEST_F(ModuleObjectTest, ConstantInputOK)
{
    const state_map const_input_quantities {{"time", input_time},
                                            {"sowing_time", input_sowing_time},
                                            {"temp", input_temp},
                                            {"tbase", input_tbase}};
    standardBML::thermal_time_linear ttl {const_input_quantities, &output};
    ttl.run();

    EXPECT_DOUBLE_EQ(output.at("TTc"), expected_output_value);
}

TEST_F(ModuleObjectTest, LiteralInputNotOK)
{
    standardBML::thermal_time_linear ttl {
                                          {{"time", 200},
                                           {"sowing_time", 100},
                                           {"temp", 25},
                                           {"tbase", 1}},
                                          &output};
    ttl.run();

    // We don't get the expected value because the module references
    // the ephemeral first argument to the module construtor.
    EXPECT_THAT(output.at("TTc"), Not(DoubleEq(expected_output_value)));
}

