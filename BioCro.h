#ifndef BIOCRO_H
#define BIOCRO_H

// Commented-out includes are included, directly or indirectly, in the
// previous uncommented-out include.
#include <framework/biocro_simulation.h> // for biocro_simulation
// #include <framework/dynamical_system.h> // for dynamical_system
// #include <framework/state_map.h> // for state_map, state_vector_map
// #include <framework/constants.h> // for math_constants::pi
#include <framework/module_factory.h> // for module_factory
// #include <framework/module_creator.h> // for mc_vector
#include <module_library/module_library.h> // for standardBML::module_library

/**
 * The public interface for the BioCro C++ library.
 */
namespace BioCro {

    /**
     * A State represents a state or partial state of a system.  For
     * example, it is used to define the initial state of a system,
     * passed as the first argument to Simulator.  A State may be
     * initialized with an initializer list of string-double pairs,
     * for example
     *
     *     State initial_state { {"position", 0}, {"velocity", 1} };
     *
     * Values of individual State variables may be accessed with
     * either the `[]` operator (if not constant) or the `at` member
     * function.  The `[]` operator may be used to set the value of a
     * variable.
     */
    using State = state_map;
    /**
     * Parameter represents a set of named values, as required in
     * defining a Simulator object.  Like State, it may be initialized
     * with an initializer list of string-double pairs.
     *
     * Values of individual parameters may be accessed with either the
     * `[]` operator (if not constant) or the `at` member function.
     * The `[]` operator may be used to set the value of a parameter.
     */
    using Parameter_set = state_map;
    /**
     * A Variable_settings object is used to hold a collection of
     * named values and is used for the input and output when running
     * a module.  Such an object is passed as the _input_quantities_
     * (first) parameter value of the `create_module` function of a
     * Module_creator object, and a pointer to such an object is
     * passed as the _output_quantities_ (second) parameter value.
     *
     * Like State and Parameter_set objects, a Variable_settings
     * object can be initialized with an initializer list of
     * string-double pairs.  But for the outputs, a more useful and
     * less tedious method is to loop through the desired variable
     * names to add them with corresponding values.  For example, if
     * `w` is a Module_creator object, we can get the names of the
     * output variables using `w->get_outputs()`.  Then, assuming
     * `outputs` is a default-initialized (empty) Variable_settings
     * object, we can add and set all of the required output variables
     * to zero using the following routine:
     *
     *     for (string param : w->get_outputs()) {
     *         outputs[param] = 0.0;
     *     }
     *
     * Then we can proceed to create our module with
     *
     *     auto module = w->create_module(inputs, &outputs);
     *
     * and run it with
     *
     *     module->run()
     *
     * We can then access the outputs with either
     *
     *     outputs[variable_name]
     *
     * or
     *
     *     outputs.at(variable_name)
     */
    using Variable_settings = state_map;
    using Variable_setting = Variable_settings::value_type;
    using Variable_names = string_vector;
    /**
     * System_drivers is used to define the drivers for a simulation
     * (the third argument of the Simulator constructor).  A
     * Variable_settings object may be initialized with an initializer
     * list consisteing of string-vector<double> pairs, and the
     * vectors themselves may be initialized with initializer lists.
     * For example, in fixture class `MultipleModuleLibrariesTest` in
     * file `test_multiple_module_libraries.cpp`, we initialize a
     * System_drivers variable using
     *
     *     BioCro::System_drivers drivers
     *         { {"time", { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }},
     *           {"temp", { 5, 8, 10, 15, 20, 20, 25, 30, 32, 40}} };
     *
     */
    using System_drivers = state_vector_map;
    using Simulation_result = state_vector_map;
    using Module_set = mc_vector;
    using Module = std::unique_ptr<module>;
    using Module_names = string_vector;
    using Standard_BioCro_library_module_factory = module_factory<standardBML::module_library>;
    using Module_creator = module_creator*;

    /**
     * A Simulator is constructed with a constructor having signature
     *
     *     Simulator(State initial_state, Parameter_set parameters, System_drivers drivers, Module_set direct_modules, Module_set differential_modules)
     *
     */
    using Simulator = biocro_simulation;
}

// This namespace is simply a kludgy way to prevent direct use of
// global names introduced by the includes above.  (This isn't an
// exhaustive list of introduced names.)
namespace {
    int state_map;
    int state_vector_map;
    int string_vector;
    int mc_vector;
    int module_creator;
    int ode_solver_factory;
    int dynamical_system;
    int biocro_simulation;
    int module;
    int ode_solver;

    template <typename T>
    class module_factory;
}

#endif
