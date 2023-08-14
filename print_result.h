#include <iostream>
#include <algorithm> // for std::max
#include "BioCro.h"

using std::cout;
using std::endl;
using std::setw;

inline void print_result(const BioCro::Simulation_result &result) {
    constexpr unsigned long minimum_width{13};  // A width of 12 is
                                                // room for a possible
                                                // sign, 6 significant
                                                // digits, a decimal
                                                // point, and a
                                                // possible exponent
                                                // (consisting of an
                                                // "e" followed by a
                                                // sign and a
                                                // two-digit number);
                                                // a width of 13
                                                // allows for a
                                                // separation space.
    std::unordered_map<string, int> field_widths{};
    for (auto item : result) {
        unsigned long field_width {std::max(item.first.length() + 1, minimum_width)};
        cout << setw(field_width) << item.first;
        // Store width for subsequent rows:
        field_widths[item.first] = field_width;
    }
    cout << endl;

    // Get an arbitrary column from the result and find its length.
    auto an_item = result.begin();
    vector<double> a_column {an_item->second};
    size_t length {a_column.size()};

    for (size_t i = 0; i < length; ++i) {
        for (auto item : result) {
            cout << setw(field_widths[item.first]) << item.second[i];
        }
        cout << endl;
    }
}
