/**
 *  Generally, this file provides somewhat lower-level exposure to
 *  BioCro classes and functions than is provided by BioCro.h.  It
 *  also provides (for now) some utility functions strictly meant for
 *  testing.  These may eventually be moved elsewhere.
 */
#ifndef BIOCRO_EXTENDED_H
#define BIOCRO_EXTENDED_H

// Commented-out includes are included, directly or indirectly, in the
// previous uncommented-out include.
#include <framework/module_factory.h> // for module_factory
#include <framework/ode_solver_library/ode_solver_factory.h> // for ode_solver_factory
// #include <framework/ode_solver.h> // for ode_solver

// This is needed only for testing the use of external module
// libraries, including testing the use of multiple module libraries:
#include "testBML/src/module_library/module_library.h" // for testBML::module_library

namespace BioCro {
    using Test_BioCro_library_module_factory = module_factory<testBML::module_library>;

    using Dynamical_system = std::shared_ptr<dynamical_system>;

    inline Dynamical_system make_dynamical_system
    (
     state_map const& initial_state,
     state_map const& parameters,
     state_vector_map const& drivers,
     mc_vector const& steady_state_modules,
     mc_vector const& differential_modules
     ) {
        return Dynamical_system(new dynamical_system(
            initial_state,
            parameters,
            drivers,
            steady_state_modules,
            differential_modules));
    }

    using Solver = std::unique_ptr<ode_solver>;

    inline Solver make_ode_solver
    (
     std::string const& ode_solver_name,
     double step_size,
     double rel_error_tol,
     double abs_error_tol,
     int max_steps
     ) {
        return Solver
            (ode_solver_factory::create
             (
              ode_solver_name,
              step_size,
              rel_error_tol,
              abs_error_tol,
              max_steps
              )
             );
    }


    ////////////////////////////////////////////////////////////////////////////
    // Some utility functions useful in testing
    ////////////////////////////////////////////////////////////////////////////
    using State = state_map;
    using Ordered_variable_list = string_vector;

    // Gets the current state of the differential variables.
    inline State get_current_state(Dynamical_system ds) {
        Ordered_variable_list keys{ds->get_differential_quantity_names()};
        auto size = keys.size();
        auto differential_quantities = vector<double>(size);
        ds->get_differential_quantities(differential_quantities);
        State current_state;
        for (auto i = 0; i < keys.size(); ++i) {
            // It matters here that the keys (as initialized by
            // ds->get_differential_quantity_names()) are in the same
            // order as the corresponding differential_quantities.
            current_state[keys[i]] = differential_quantities[i];
        }
        return current_state;
    }

    using Simulation_result = state_vector_map;

    inline size_t get_result_duration(Simulation_result result) {
        auto a_column = result.begin();
        return (a_column->second).size();
    }

    // Gets the state of all quantities in a particular row of a result.
    inline State get_state_from_result(Simulation_result result, size_t row_number) {
        State state;
        for (auto column : result) {
            auto name = column.first;
            auto values = column.second;
            state[name] = values.at(row_number);
        }
        return state;
    }

    // Get the initial state of all quantities in a result.
    inline State get_initial_result_state(Simulation_result result) {
        return get_state_from_result(result, 0);
    }

    // Get the final state of all quantities in a result.
    inline State get_final_result_state(Simulation_result result) {
        return get_state_from_result(result, get_result_duration(result) - 1);
    }

    /**
     * Unlike Variable_settings, which encompasses a set of variables
     * together with their values, Variable_set is simply a set of
     * variable names.
     */
    using Variable_set = std::set<std::string>;

    // Get the keys of a mapping (e.g., a State or a System_drivers
    // specification) as a set.
    template<typename Mapping>
    inline Variable_set keys(Mapping mapping) {
        Variable_set keys;
        std::transform(mapping.begin(), mapping.end(),
                       std::inserter(keys, keys.end()),
                       [](auto pair){ return pair.first; });
        return keys;
    }
}
#include "BioCro.h"

#endif
