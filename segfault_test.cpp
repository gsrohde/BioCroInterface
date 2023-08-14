// From https://stackoverflow.com/questions/47583352/how-to-catch-segmentation-fault-with-google-test
//
// I've made slight modifications and elaborations.  Note these test
// have nothing per se to do with BioCro.  They are mainly here for
// reference.

#include <gtest/gtest.h>

int deref(int* p)
{
    return *p;
}


// This is the sample in the original post, with deref playing the
// role of "foo", nullptr playing the role of "nullParameter", and the
// second (empty string) argument provided to fullfill the signature
// requirements of EXPECT_DEATH.
TEST(original_post, will_segfault)
{
    EXPECT_DEATH(deref(nullptr), "");
}

TEST(test_deref_1, will_segfault)
{
    ASSERT_EXIT((deref(nullptr), exit(0)),
                ::testing::KilledBySignal(SIGSEGV),
                ".*");
}

TEST(test_deref_2, will_not_segfault)
{
    int i = 42;
    ASSERT_EXIT((deref(&i), exit(0)), ::testing::ExitedWithCode(0), ".*");
}

// This test will fail but not crash the testing framework.
TEST(test_deref_2_modified, DISABLED_will_not_segfault)
{
    // deref(nullptr); <-- This would crash the framework.
    ASSERT_EXIT((deref(nullptr), exit(0)), ::testing::ExitedWithCode(0), ".*");
}

// We get here because wrapping the deref(nullptr) inside ASSERT_EXIT
// prevents the framework from crashing.
TEST(bogus_test, test_framework_has_not_crashed)
{
    ASSERT_EQ(5, 5);
}
