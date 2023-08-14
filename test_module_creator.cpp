// The tests in this file test the retrieval of module creation
// objects and the behavior of the objects retrieved.

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "BioCro.h"

constexpr char known_module_name[] {"harmonic_oscillator"};
constexpr char known_module_inputs[] {"position|velocity|mass|spring_constant"}; // as a regexp
constexpr char known_module_outputs[] {"position|velocity"}; // as a regexp
constexpr char bogus_module_name[] {"bogus"};

using testing::MatchesRegex;

using Module_factory = BioCro::Standard_BioCro_library_module_factory;

TEST(ModuleCreatorTest, KnownModule) {
    BioCro::Module_creator creator;

    // Test that we don't get an exception when we try to retrieve a
    // module known to be part of the Standard BioCro Module Library:
    ASSERT_NO_THROW({
            creator = Module_factory::retrieve(known_module_name);
        });
    // Ensure that the name returned by the creator matches the name
    // used in retrieving it:
    ASSERT_EQ(creator->get_name(), known_module_name);

    BioCro::Variable_names inputs = creator->get_inputs();
    BioCro::Variable_names outputs = creator->get_outputs();

    // Check that there are 4 inputs and that they have the expected
    // names.
    EXPECT_EQ(inputs.size(), 4);
    for (std::string& item : inputs) {
        EXPECT_THAT(item, MatchesRegex(known_module_inputs));
    }

    // Check that there are 2 outputs and that they have the the
    // expected names.
    EXPECT_EQ(outputs.size(), 2);
    for (std::string& item : outputs) {
        EXPECT_THAT(item, MatchesRegex(known_module_outputs));
    }
}


TEST(ModuleCreatorTest, BogusModule) {
    // Test that we get an exception when we try to retrieve a module
    // that doesn't exist.
    ASSERT_THROW({
            Module_factory::retrieve(bogus_module_name);
        }, std::out_of_range);
}
