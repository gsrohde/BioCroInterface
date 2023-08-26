// Compile with the flag -DVERBOSE=true to get verbose output.  This
// will mainly be useful for debugging the read_data_file function.
#ifndef VERBOSE
#define VERBOSE false
#endif

#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
using namespace std;

// This isn't declared in module_library/AuxBioCro.h (or any other
// header file) so we just declare it here:
double poisson_density(int x, double lambda);

// An ad hoc function for reading in Poisson density values from a
// file having a very specific format.
void read_data_file(vector<double>& lambda_values,
                    vector<vector<double>>& density_values)
{
    std::ifstream data_file("table_of_poisson_probabilities.txt");

    string line {};      // stores the last line read
    istringstream sb {}; // stores the line as a stream to read from

    // Keep track to the number of the first column in the current
    // column group:
    int start_column {};

    // For reading integers corresponding to values of X.  We never
    // use these read values except in a sanity check and for debug
    // output:
    int x {};

    while (data_file) {

        getline(data_file, line);
        
        if (VERBOSE) cout << "\ngot line: " << line << endl;

        // skip empty lines
        if (line.length() == 0) continue;

        sb.str(line);

        int first_char {sb.peek()};

        if (first_char == '#') {
            // a comment line; ignore
            continue;
        }

        if (first_char == 'L') {
            if (VERBOSE) {
                cout << "first_char is " << static_cast<char>(first_char) << endl;
                cout << "marker for new set of lambda values: ";
                cout << sb.str() << endl;
            }

            // Get ready for a new batch of columns:
            start_column = lambda_values.size();

            continue;
        }

        if (first_char == 'X') {
            // We're on a header line consisting of lambda values.

            if (VERBOSE) {
                cout << "first_char is " << static_cast<char>(first_char) << endl;
                sb.ignore(2, ' ');
                string remainder {};
                getline(sb, remainder);
                cout << "the lambda values themselves: " << remainder << endl;
                sb.seekg(0); // Put things back to the way they were.
            }

            sb.ignore(1, 'X');

            double lambda {};
            sb >> lambda;
            while (sb) {
                lambda_values.push_back(lambda);
                // Add a new column for each new lambda value:
                density_values.push_back({});

                // Try to read the next lambda value:
                sb >> lambda;
            }
            sb.clear(); // Clear the error that caused the loop to break.
            continue;
        }

        // If we get here, we're on a line of data values.

        if (VERBOSE) cout << "first_char is " << static_cast<char>(first_char)
                          << endl; // i.e., not '#', 'L', or 'X'

        if (VERBOSE) {
            // The first number is the row heading, giving the value of X
            // for the row.
            sb >> x;
            if (sb) {
                cout << "X value: " << x << endl;
            }
            else {
                // Every non-blank line not starting with '#', 'L', or
                // 'X' *should* start with an integer.
                cout << "What's up?  Something's wrong." << endl;
            }
            // skip over space before printing the remainer of the
            // line:
            sb.ignore(1, ' ');
            string remainder {};
            getline(sb, remainder);
            cout << "the density values: " << remainder << endl;
            sb.seekg(0); // Put things back to the way they were.
        }

        // The first number is the row heading, giving the value of X
        // for the row.
        sb >> x;

        int i = start_column; // The first column of the current batch.

        double value{};
        sb >> value;
        while (sb) {

            // We don't use the value read into x except as part of
            // this sanity check:
            ASSERT_EQ(x, density_values[i].size()) << "Something's wrong." << endl;

            density_values[i].push_back(value);
            ++i;
            sb >> value;
        }
        sb.clear(); // Clear the error that caused the loop to break.
    }
}

class PoissonDensityTest : public ::testing::Test {
protected:
    PoissonDensityTest() {
        read_data_file(lambda_values, density_values);
    }

    vector<double> lambda_values {};
    vector<vector<double>> density_values {};
};

// The eponymous table is from
// https://ux1.eiu.edu/~aalvarado2/levine-smume6_topic_POIS.pdf.
TEST_F(PoissonDensityTest, CompareWithTable) {

    // Calculated values *should* match the table values up to the
    // rounding error (half of the smallest difference in the fourth
    // decimal place, i.e. 0.0001/2 = 0.00005).  In point of fact, due
    // to incorrect rounding of some of the values in the table, we
    // have to increase the tolerance slightly, to 0.000055.
    double tolerance {5.5e-5};

    for (int i = 0; i < density_values.size(); ++i) {
        double lambda {lambda_values.at(i)};

        vector<double> column {density_values.at(i)};

        for (int j = 0; j < column.size() && j < 110; ++j) {

            // Note that because in each column, x starts at 0 and
            // increases by one in each following row, x and the index
            // j always correspond (see the sanity test in the
            // read_data_file function above).

            // There are two significant misprints in the table we
            // use:
            //
            // The value for f(3.2, 1) is given as 0.1340; it should
            // be 0.1304.
            //
            // The value for f(6.8, 12) is given as 0.0277; it should
            // be 0.0227.
            if ((i == 31                // lambda = 3.2 when i = 31
                 && j == 1) ||
                (i == 67                // lambda = 6.8 when i = 67
                 && j == 12)) continue; // skip these bad values

            EXPECT_NEAR(poisson_density(j, lambda), column.at(j), tolerance) <<
                "BAD RESULT FOR lambda = " << lambda << ", X = " << j;
        }
    }
}


// The sum of density values over all x for a given value of lambda
// should be 1.  For values of lambda up to 20, adding the density
// values for the first 40 values of x suffices to yield a sum
// approximately equal to 1 to within 4 decimal places.
TEST(PoissonDensitySanityTest, SumsToOne) {
    for (double lambda = 0.1; lambda <= 20; lambda += 0.1) {
        double sum {};
        for (int x = 0; x < 40; ++x) {
            sum += poisson_density(x, lambda);
        }
        EXPECT_NEAR(sum, 1.0, 5e-5) << "BAD RESULT FOR lambda = " << lambda;

        // Not only should the sum be close to 1 if we add enough of
        // the terms, it should definitely not be greater than one.
        // But we allow that, due to rounding errors, the sum may end
        // up being very slightly greater than 1:
        EXPECT_LE(sum, 1.0 + 4e-16) << "BAD RESULT FOR lambda = " << lambda;
    }
}

