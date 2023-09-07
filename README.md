# mex-integration
Framework for fast integration in MATALB, geared towards astrodynamics applications.  The src/include folder provides header files that make integration with event finding using MEX easy.  The example .cpp file in src can be used as a framework for building your own mex functions.

## Installation Instructions
1. Clone the repository, or copy the files in the src/include folder.  These are the header files that make up the library.
2. Ensure that you have downloaded boost and make note of the BOOST_ROOT directory (typically something like boost_1_81).

## Visual Studio Setup
Follow the instructions in WindowsInstructions.md to define a property sheet for mexing.  Be sure to add the src/include folder to your include path.

## Example Project - Integrate to an XZ Plane Crossing in the CR3BP



# Pitfalls
## BOOST vs. MATLAB Error Tolerancing
Boost and matlab employ different tolerancing strategies in their integrators.  Specifically, BOOST keeps an integration step if 
$$e_i \leq \varepsilon_{abs}+\varepsilon_{rel}(a_1|\bar{x}|+a_2|\frac{d\bar{x}}{dt}|dt)$$
whereas MATLAB integrators keep an integration step if
$$e_i \leq \max(\varepsilon_{abs}, \varepsilon_{rel}|\bar{x}|)$$

This can lead to MATLAB taking many more integration steps, especially if variational equations are propagated.  It is possible to set the values of $a_1$ and $a_2$ in BOOST, but by default they are set to 1.  Documentation for BOOST can be found [here](https://www.boost.org/doc/libs/1_81_0/libs/numeric/odeint/doc/html/boost_numeric_odeint/tutorial/harmonic_oscillator.html) (where tolerances are discussed in the section on "Integration with Adaptive Step") and documentation for MATLAB [here](https://www.mathworks.com/help/simulink/ug/variable-step-solvers-in-simulink-1.html).
## Using Debug configuration
If you build your solution using objects compiled with the Debug specification, mexing may not work correctly.  Recompile your code using a Release configuration.

## VS Compiler Warnings
Often, the Visual Studio compiler will give warnings when compiling your mex code.  Do not be alarmed, some parts of the mexApiAdapterImpl.hpp necessary for mexing do things that the Visual Studio compiler does not like.  One common warning is `C26478 Don't use std::move on constant variables`.

## Build to .dll vs .lib
Note that a .lib build does not work for mexing.  A .dll build is required as specified in Visual Studio.
