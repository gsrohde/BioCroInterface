_For instructions on using these materials, see [README](README)
(online at [README on
GitHub](https://raw.githubusercontent.com/gsrohde/BioCroInterface/master/README))._

# Towards a C++ Interface for BioCro

For some time now, we have bandied about the idea that the BioCro C++
library should be able to be used on its own without the R interface
provided by the BioCro R package.  The [BioCro
II](https://academic.oup.com/insilicoplants/article/4/1/diac003/6527687)
paper even states "The central framework can conveniently be accessed
through the R package interface or directly through C/C++, and other
interfaces can be developed without duplicating the essential code."
It is conveniently ambiguous here whether "conveniently" applies both
to "[access] through the R package interface" and "directly through
C/C++" or only to the former.  From the discussion that follows, it
seems clear that this purported convenience is currently limited to R
users.  So I have been thinking about what a public interface to the
C++ library should look like.  Concomitantly, I have been
experimenting with testing BioCro at the C++ level using the
_GoogleTest_ testing framework.

### Public interface versus publicly-available classes and functions

The number of public classes and member functions in the BioCro C++
library is vast, but the number of R functions that interact with this
library is relatively small—only about ten.  Clearly, a public
interface to the BioCro C++ library need only make available a tiny
fraction of the classes and functions that could be called by a user
of the library.

### An experimental Public Interface/Test Suite repository

To test out the design of a public interface, I've created a public
repository `gsrohde/BioCroInterface` on GitHub.  This consists of a few
public-interface header files, several tests, a _Make_ file for
building the tests, a _Doxyfile_ for configuring builds of the Doxygen
documentation, and a submodule, `testBML`, used in testing the use of
user-created module libraries.  Also included is a README file that
tells how to use the repository's collection of files with BioCro; and
finally, `README.md`, the file that you are reading.

## Notes on the interface files

There are two main interface header files, `BioCro.h` and
`BioCro-extended.h`.

`BioCro.h` is intended to provide only the highest-level classes and
data structures needed to use `BioCro.so` in writing C++ programs.  To
wit, it provides the class most central to BioCro’s functioning,
called here `BioCro::Simulator`, and it provides the data types needed
in constructing an object of this class:

* `State` (for specifying the initial state)
* `Parameter_set`
* `System_drivers`
* `Module_set` (for both the direct and the differential module collections)
* `Standard_BioCro_library_module_factory` (for constructing a `Module_set`)

It also declares a name `Simulation_result` for the type returned by
`Simulator.run_simulation`.

Note that all of these names belong to a namespace called `BioCro`,
but we won't always include the `BioCro::` prefix in the discussion
that follows.  Putting the names in a namespace, in addition to
circumventing potential (if unlikely) name clashes, helps to emphasize
where these names come from.

I have chosen these names to further distance conceptual types
embodied by these classes from their implementation.  For example,
both`State` and `Parameter_set` are implemented by the standard
library type `unordered_map<string, double>` (aliased to `state_map`
in BioCro).  But I wanted to avoid incorporating hints about their
implementation ("map") into their names; furthermore, they seemed
conceptually distinct enough to give them _different_ names.  (For
further discussion along these lines see the [C++ Core
Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rl-name-type).)

As another example, I use the name `Module_set` to emphasize that the
order of the elements used in initializing objects of this class is
immaterial.  (It is also meant to suggest that any given module
appears at most once in a `Module_set`; but this isn’t enforced in the
case of sets of differential modules: `Module_set` is in fact
implemented as a `std::vector`, which has order and doesn't enforce
uniqueness.)  Here, I am thinking of "set" as a mathematical concept.
I am not thinking of the associative container `set` available from
the standard C++ library.  (The two are, of course, related, and
probably more so than `std::vector` is related to the vectors used in
mathematics.)

One drawback of using these concept-oriented names is that they give
the user little clue as to how objects of such classes may be used—in
other words, what member functions are available when working with
such objects: Most users know roughly what member functions are
available when confronted with an object of type `vector<string>`, for
example.  But present them with an object with a type called
`Module_names` (which, as it happens, is aliased to `vector<string>`)
and they won't have a clue as to how to work with such an object
unless you spell it out.

The solution is to document each of these aliases as if they
were classes defined directly.  I have sketched out a few
Doxygen-style comments to hint at what such documentation might look
like.  But since this is only a rough draft of an interface, I have
stopped short of anything approaching comprehensive user-oriented
documentation.

`BioCro_Extended.h` provides slightly lower-level classes
(vis. `Dynamical_system` and `Solver`), as well as some convenience
functions for obtaining objects of such classes.  (It also provides
some utility functions mainly useful in writing the tests.)  Note that
`Dynamical_system` is aliased to `std::shared_ptr<dynamical_system>`
rather than simply to `dynamical_system`, thus shielding the user from
some of the messier details of working with the `BioCro.so` library.
But of course, to make this name from the interface usable, we need to
provide complete usage information: for example, we should tell the
user that `Dynamical_system` objects have a member function called
`get_differential_quantity_names()`, we should describe what that
function does, and we should make clear that it must be called on a
`Dynamical_system` object `ds` with  
`ds->get_differential_quantity_names()` rather than with
`ds.get_differential_quantity_names()`.


### Preventing direct use of exposed BioCro classes

To encourage using only the names provided by the interface files, I
use a somewhat kludgy hack involving the use of an anonymous
namespace: Following the _using_ statements in `BioCro.h`, I redefine
the exposed names brought in by the _include_ statements at the top of
`BioCro.h`.  This causes no conflict with the names as originally
defined unless the subsequent code actually _uses_ one of these names.
For example, if we try to use `state_map` in a source file that
includes `BioCro.h`, we will get a compilation error, something like
**"error: reference to 'state_map' is ambiguous"**.

Arguably, the anonymous namespace should redefine _all_ names brought
in by the includes at the top of `BioCro.h`, thus preventing any
direct use of the functions and classes so exposed.  But I didn't try
to make an exhaustive list and instead concentrated on those names
most likely to be used "by mistake."

### Going beyond aliasing

Note that names declared in the interface header files are mostly
aliases.  Thus, the class functions available for classes declared in
these header files are exactly the same as the class functions of the
class aliased.  But we could have taken a further step in distancing
the public interface types from their BioCro implementations: we could
have, for example, made the names into names of classes that wrap
existing class instead of merely aliasing them.  In this way, we could
restrict, if desired, the functions available, or even rename or alter
them in some useful way.

One example where we have done this is the `Single_use_simulator`
class, defined in the file `safe_simulators.h`.  Unlike `Simulator`,
which is merely an alias for BioCro's `biocro_simulation` class,
`Single_use_simulator` presents a modified version of that class in
which `run_simulation` is only allowed to be run once.  (This prevents
possibly erroneous usages of the simulator class.)

A different approach is used by the `Alternative_idempotent_simulator`
class, also defined in `safe_simulators.h`.  Whereas
`Single_use_simulator` inherits from `Simulator`
(a.k.a. `biocro_simulation`) and merely redefines its `run_simulation`
member function to be "safe", `Alternative_idempotent_simulator` uses,
but does not inherit from, the `Simulator` class.  Instead, it creates
a `Simulator` object on the fly whenever its `run_simulation` function
is called, and it delegates to _that_ object's `run_simulation`
function.  This avoids the problem of possibly running
`run_simulation` when the simulator is in a _dirty_ state.

A third approach is taken by `Idempotent_simulator`.  This class
merely copies most of the code in the original `Simulator`
(`biocro_simulation`) class definition but includes a call to the
underlying dynamical system's reset function at the top of the
`run_simulation` function, ensuring the system is in a clean state
before a simulation is run.  (This is perhaps what `biocro_simulation`
itself should do.)

## The tests

The focus of the _GoogleTest_ tests is primarily to demonstrate the
use of the proposed interface, both how it should be used and how it
can be misused.  In particular, these tests demonstrate some of the
pitfalls that await the unwary—pitfalls that R users are largely
insulated from—and possible methods of ameliorating such pitfalls.

It also demonstrates features of _GoogleTest_ itself and various ways
in which it might be used.

Note that if _Make_ is run with the variable setting `VERBOSE=true`,
many of the tests will display additional output.  For example, if we
build `test_repeat_runs` with

    make test_repeat_runs VERBOSE=true

and then run it with

    ./test_repeat_runs --gtest_also_run_disabled_tests \
                       --gtest_filter=BiocroSimulationTest.DISABLED_runSimulationIsIdempotent

we can see how the `Simulator` object resets the `time` variable back
to 0 upon starting the second run, but the `TTc` variable keeps the
value it ended up with from the first run.

(Note that if the executable file is up to date, you will have to use
_Make_'s `-B` flag to force a rebuild when turning verbosity on or
off.)

Here are a few notes on the individual test files:

* `segfault_test.cpp` (build and run with `make 1`)

   These tests merely demonstrate how _GoogleTest_ can run code that
   causes a segmentation fault without itself crashing, and how it can
   detect segmentation faults.

* `run_test_simulator.cpp` (build and run with `make 10`)

   This file merely demonstrates basic usage of the `Simulator` class.

* `test_dynamical_system.cpp` (build and run with `make 2`)

   These tests test the various class functions available from a
   `BioCro::Dynamical_system` object.

   Many users will be content to use only `BioCro::Simulator` objects.
   `BioCro::Dynamical_system` objects might be considered somewhat
   lower level and thus this name is declared in `BioCro_Extended.h`
   rather than in `BioCro.h`.  One possible use of
   `BioCro::Dynamical_system` objects is to be able to easily solve
   one system using a variety of solvers without having to define a
   new simulator object each time.

* `test_harmonic_oscillator.cpp` (build and run with `make 3`)

   This file tests a `Simulator` object based upon a well-known and
   well-studied type of dynamical system, one having an explicit
   mathematical solution.  The tests show that the simulation results
   match the expected behavior, given an assortment of
   randomly-assigned values for the input parameters and initial
   state.

   The tests in this file probably come closest to being true
   regression tests.

* `test_module_creator.cpp` (build and run with `make 4`)

   These tests test the retrieval of `BioCro::Module_creator` objects
   using `BioCro::Standard_BioCro_library_module_factory`'s `retrieve`
   function and the use of a creator's various functions, once such an
   object has been retrieved.

* `test_module_evaluation.cpp` (build and run with `make 5`)

   The tests in the file demonstrate how to use a BioCro module
   directly, outside the context of a simulator or a dynamical system,
   using classes provided by the interface defined in `BioCro.h`.  It
   also demonstrates a pitfall arising from careless usage of the
   interface.

* `test_module_factory_functions.cpp` (build and run with `make 6`)

   This file tests the public member functions of module factory
   objects except for the most important one (`retrieve`), which is
   tested elsewhere.

* `test_module_object.cpp` (build and run with `make 7`)

   Here we demonstrate accessing classes from `BioCro.so` directly,
   not through the `BioCro.h` interface, to construct and run a BioCro
   module directly.  (Compare with `test_module_evaluation`.)  Again,
   it shows the problems that arise when a module is not constructed
   correctly.

* `test_multiple_module_libraries.cpp` (build and run with `make 8`)

   Here, using the standard BioCro module library together with a
   user-defined module library is demonstrated.

* `test_repeat_runs.cpp` (build and run with `make 9`)

   The tests in this file demonstrate a quirk in objects of class
   `Simulator` whereby when they are run a second time, the drivers
   are set back to their initial values but the differential
   quantities are not.  It tests out various alternative versions of a
   simulator that protect against this problem.

To compile all of the tests into one file and run them, call

    make run_all_tests

(or just `make`, since `run_all_tests` is the default target).