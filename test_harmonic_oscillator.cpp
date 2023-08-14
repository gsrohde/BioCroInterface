// Compile with the flag -DVERBOSE=true to get verbose output.
#ifndef VERBOSE
#define VERBOSE false
#endif

#include <gtest/gtest.h>

#include <cmath>

#include "BioCro.h"

#include "Random.h"
#include "print_result.h"

using namespace std;
using math_constants::pi;
using Module_provider = BioCro::Standard_BioCro_library_module_factory;

/*
 * This tests simulation of an undamped harmonic oscillator consisting
 * of an object with mass m suspended on a spring with spring constant
 * k.  The position is the vertical displacement from the equilibrium
 * position of the object so that if the object's position and
 * velocity are both zero, the object remains at rest.
 *
 * The oscillation should obey the formula
 *
 *     x(t) = A sin(ωt + φ),
 *
 * where x(t) is the position at time t, A is the amplitude of the
 * oscillation, ω is proportional to the oscillation frequency f (ω =
 * 2πf), and φ is the phase.  We can use
 *
 *     x(0) = A sin(φ) = initial_state["position"]
 *
 * and
 *
 *     v(0) = Aω cos(φ) = initial_state["velocity"]
 *
 * together with the force equations
 *
 *     F(t) = m a(t) = m (-Aω² sin(ωt + φ)) = m (-ω² x(t))
 *
 * and
 *
 *     F(t) = -k x(t)
 *
 * to solve for the parameters A, ω, and φ in terms of m, k, and the
 * initial state (x(0), v(0)).
 *
 * Note that the units for the quantities in these equations are
 * unspecified but must be consistent with one another.  In the system
 * we set up below, there are five quantities specified in setting up
 * the system: a time ("timestep"), a mass ("mass"), a length
 * ("position"), a velocity ("velocity"), and a force per unit length
 * ("spring_constant").  An additional quantity, energy, appears in
 * the output (as "kinetic_energy", "spring_energy", and
 * "total_energy").  Moreover, acceleration appears as a quantity
 * behind the scenes when we use the "harmonic_oscillator" module to
 * compute the change in velocity.
 *
 * If we use coherent SI units for all quantities used in setting up
 * the system--seconds, kilograms, meters, meters per second, and
 * newtons per meter--then everything works out.  The
 * harmonic_oscillator computes the change in position (x) and
 * velocity (v) using the equations
 *
 *     Δx/Δt = v
 *
 *     Δv/Δt = -k x / m
 *
 * So if x is in meters, Δt (timestep) is in seconds, and v is in
 * meters per second, the units work out in the first equation.  And
 * if k (the spring constant) has units of kilograms per second
 * squared, and m (the mass) has units of kilograms, then the units in
 * the second equation work out as well.  Moreover, using the formulas
 * in the harmonic_energy module, the energy units will turn out in
 * kilogram-meters squared per second squared, that is, in joules.
 *
 * But suppose we assume the timestep to be in units of hours, an
 * assumption made in all of the biologically-oriented BioCro modules.
 * If we keep meters as the unit of length, then we must assume the
 * velocity v is expressed in meters per hour.  Then if the mass m is
 * still expressed in kilograms, the spring constant k must be in
 * units of kilograms per hour squared. Even more awkwardly, the
 * energy units will be in kilograms-meters squared per hour squared.
 * Thus, for example, if the total energy turns out to be 1 unit, this
 * must be interpreted as equivalent to approximately 7.716E-8 joules.
 *
 * In rare cases, where the period ends up being very short, the tests
 * may fail.
 */
class HarmonicOscillator_Test : public ::testing::Test {
   protected:

    void set_number_of_timesteps(int n) {
        vector<double> times;
        for (size_t i{0}; i <= n; ++i) {
            double time{i * timestep()};
            times.push_back(time);
        }
        drivers = { { "elapsed_time", times } };
    }

    // This is the total number of steps.
    int number_of_timesteps() const {
        return drivers.at("elapsed_time").size() - 1;
    }

    double omega() const {
        return sqrt(k/m);
    }

    double period() const {
        return 2 * pi / omega();
    }

    // φ (phi)
    double phase() const {
        return atan2(omega() * x0, v0);
    }

    // The time (0 or later) when the object first reaches position
    // zero.
    double first_zero_point() {
        return (phase() <= 0) ? -phase() / omega()
                              : (pi - phase()) / omega();
    }

    // A
    double amplitude() const {
        if (abs(sin(phase())) > abs(omega() * cos(phase()))) {
            return x0 / sin(phase());
        } else {
            return v0 / (omega() * cos(phase()));
        }
    }

    // Used in computing appropriate tolerance value for position near
    // zero below.
    double maximum_velocity() {
        return amplitude() * omega();
    }

    double timestep() {
        return delta_t;
    }

    double duration() {
        return number_of_timesteps() * timestep();
    }

    BioCro::Simulation_result get_simulation_result() const {
        return get_simulator().run_simulation();
    }

   private:

    // By making this private, we make sure that we recreate the
    // simulation every time we run it (via get_simulation_result()).
    // This way, the simulation can't get into a bad state (e.g. by
    // resetting the drivers variable via set_number_of_timesteps())
    // between the time we create it and the time we run it.

    BioCro::Simulator get_simulator() const {
        return BioCro::Simulator {
            {{"position", x0}, {"velocity", v0}},
            {{"mass", m}, {"spring_constant", k}, {"timestep", delta_t}},
            drivers,
            direct_modules,
            differential_modules,
            //"boost_rosenbrock", // This gives odd results if
                                  // number_of_timesteps() = 1,
                                  // appearing to show no change
                                  // in state from time 0 to the
                                  // subsequent time.
            "boost_rk4",          // This and boost_rkck54 seem to work the best here.
            //"boost_rkck54",
            //"auto",             // Chooses Rosenbrock in this case.

            //"boost_euler",      // The Euler solvers perform
                                  // extremely poorly, showing the
                                  // total energy climbing from 5
            //"homemade_euler",   // to about 1,352,000.
            1,
            0.0001,
            0.0001,
            200
        };
    }

    const double delta_t {0.01};

    BioCro::System_drivers drivers { {"elapsed_time",  { 0, 1 }} };

    BioCro::Module_set direct_modules
        { Module_provider::retrieve("harmonic_energy") };
    BioCro::Module_set differential_modules
        { Module_provider::retrieve("harmonic_oscillator") };

    const Rand_double double_gen { -10, 10 };
    const Rand_double pos_double_gen { 1e-5, 100 };

 protected:
    // We want these accessible to derived classes (vis. the tests
    // themselves), but they must be declared after double_gen and
    // pos_double_get because those random number generators must be
    // initialized first.
    const double x0 {double_gen()};
    const double v0 {double_gen()};
    const double m {pos_double_gen()};
    const double k {pos_double_gen()};
};

template <typename T> int sgn(T val) {
    return (T{0} < val) - (val < T{0});
}

// Check that the object position returns to zero every half period
// and passes from a positive to a negative value (or vice versa)
// during the step that crosses the half-period time point.
TEST_F(HarmonicOscillator_Test, PeriodIsCorrect) {

    if (VERBOSE) cout << "initial position: " << x0 << endl;
    if (VERBOSE) cout << "initial velocity: " << v0 << endl;
    if (VERBOSE) cout << "mass: " << m << endl;
    if (VERBOSE) cout << "spring constant: " << k << endl;
    if (VERBOSE) cout << "amplitude: " << amplitude() << endl;
    if (VERBOSE) cout << "period: " << period() << endl;
    if (VERBOSE) cout << "phase: " << phase() << endl;

    set_number_of_timesteps(floor(period()/timestep() * 5) + 1);
    // We want to inspect the values both before (or at) and after the
    // time point marking the end of the final period, hence the "+ 1".

    if (VERBOSE) cout << "number of timesteps: " << number_of_timesteps() << endl;
    if (VERBOSE) cout << "size of timestep: " << timestep() << endl;
    if (VERBOSE) cout << "duration: " << duration() << endl;

    BioCro::Simulation_result result {get_simulation_result()};
    if (VERBOSE) print_result(result);

    // position should return to zero every half period.
    // It should change sign as well.
    for (double time = first_zero_point();

         time < number_of_timesteps() * timestep();
         // number_of_timesteps() is the maximum allowable index, so
         // if x = time/timestep(), ensure floor(x) + 1 (used as an
         // index below) is less than or equal to
         // number_of_timesteps().

         time += period()/2) {

        double x = time/timestep();
        int i = round(x);
        EXPECT_NEAR(result["position"][i], 0.0, maximum_velocity() * timestep())
            << "At index " << i << " position is " << result["position"][i];
        double prior_position {result["position"][floor(x)]};
        double subsequent_position {result["position"][floor(x) + 1]};
        EXPECT_TRUE(sgn(prior_position) != sgn(subsequent_position));
        if (VERBOSE) cout << "Near time = " << time
                          << ", the position changes from "
                          << prior_position << " to " << subsequent_position
                          << "." << endl;
    }

    // The maximum displacement achieved in each direction should equal the amplitude, provided that
    // duration() >=  3/4 period()
    if (4 * duration() >= 3 * period()) {
        double maximum {0};
        double minimum {0};
        for (size_t i = 0;
             i <= number_of_timesteps(); // Again,
                                         // number_of_timesteps() is
                                         // the maximum allowable
                                         // index
             ++i) {
            maximum = max(maximum, result["position"][i]);
            minimum = min(minimum, result["position"][i]);
        }
        EXPECT_NEAR(maximum, amplitude(), amplitude() * 3e-3);
        EXPECT_NEAR(minimum, -amplitude(), amplitude() * 3e-3);
    }

    // total energy should be constant
    vector<double> E = result["total_energy"];
    double init_E {E[0]};

    for (size_t i = 0; i <= duration(); ++i) {
        EXPECT_NEAR(E[i], init_E, init_E * 9e-4);
    }

}
