#ifndef SAFE_SIMULATORS_H
#define SAFE_SIMULATORS_H

#include "BioCro_Extended.h"

namespace BioCro {

// Here, we define a version of Simulator that automatically resets
// the dynamical system member object before running.  Note that we
// don't bother including the generate_report() member function.
class Idempotent_simulator
{
   public:
    Idempotent_simulator(
        // parameters passed to dynamical_system constructor
        BioCro::State const& initial_state,
        BioCro::Parameter_set const& parameters,
        BioCro::System_drivers const& drivers,
        BioCro::Module_set const& direct_mcs,
        BioCro::Module_set const& differential_mcs,
        // parameters passed to ode_solver_factory::create
        std::string ode_solver_name,
        double output_step_size,
        double adaptive_rel_error_tol,
        double adaptive_abs_error_tol,
        int adaptive_max_steps)
    {
        // Create the system
        sys = make_dynamical_system(initial_state, parameters,
                                 drivers, direct_mcs,
                                 differential_mcs);

        // Create the ode_solver that will be used to solve the system
        system_solver =
            make_ode_solver(
                    ode_solver_name,
                    output_step_size,
                    adaptive_rel_error_tol,
                    adaptive_abs_error_tol,
                    adaptive_max_steps);
    }

    BioCro::Simulation_result run_simulation()
    {
        sys->reset();
        return system_solver->integrate(sys);
    }

   private:
    BioCro::Dynamical_system sys;
    BioCro::Solver system_solver;
};

// An alternative to mimicking Simulator and having to deal with the
// underlying dynamical system and solver objects merely to be able to
// access the dynamical system's reset function is to store the
// paramter values used in making a Simulator object and simply
// remaking the simulator each time we want to run it.
class Alternate_idempotent_simulator
{
   public:
    Alternate_idempotent_simulator(

        BioCro::State const& initial_state,
        BioCro::Parameter_set const& parameters,
        BioCro::System_drivers const& drivers,
        BioCro::Module_set const& direct_mcs,
        BioCro::Module_set const& differential_mcs,

        std::string ode_solver_name,
        double output_step_size,
        double adaptive_rel_error_tol,
        double adaptive_abs_error_tol,
        int adaptive_max_steps)

        :

        initial_state{initial_state},
        parameters{parameters},
        drivers{drivers},
        direct_mcs{direct_mcs},
        differential_mcs{differential_mcs},

        ode_solver_name{ode_solver_name},
        output_step_size{output_step_size},
        adaptive_rel_error_tol{adaptive_rel_error_tol},
        adaptive_abs_error_tol{adaptive_abs_error_tol},
        adaptive_max_steps{adaptive_max_steps} {}

    BioCro::Simulation_result run_simulation()
    {
        Simulator sim {
            initial_state,
            parameters,
            drivers,
            direct_mcs,
            differential_mcs,
            ode_solver_name,
            output_step_size,
            adaptive_rel_error_tol,
            adaptive_abs_error_tol,
            adaptive_max_steps
        };
        return sim.run_simulation();
    }

   private:
    BioCro::State const& initial_state;
    BioCro::Parameter_set const& parameters;
    BioCro::System_drivers const& drivers;
    BioCro::Module_set const& direct_mcs;
    BioCro::Module_set const& differential_mcs;
    std::string ode_solver_name;
    double output_step_size;
    double adaptive_rel_error_tol;
    double adaptive_abs_error_tol;
    int adaptive_max_steps;
};

// Here we can just delegate to Simulator since we don't need to
// access Simulator's (private) Dynamical_system member.  As a bonus,
// we automatically inherit the generate_report() member function.
class Single_use_simulator : public Simulator
{
    using Simulator::Simulator; // inherit constructor

   public:
    BioCro::Simulation_result run_simulation()
    {
        if (has_been_run) {
            throw std::runtime_error("A Single_use_simulator can only be run once.");
        }
        has_been_run = true;
        return Simulator::run_simulation();
    }

   private:
    bool has_been_run {false};
};

}

#endif
