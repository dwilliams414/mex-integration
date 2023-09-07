/* Mex Function Example 
	Call in MATLAB as 

	[t, x, te, xe, ie] = <mexFunctionName>(x0, [t0 tf], system_mu, n_cross) or
	[t, x, te, xe, ie] = <mexFunctionName>(x0, [t0 tf], system_mu, n_cross, absTol, relTol, initialStep)

	The second form must be used if tf < t0, to allow for a negative initial step in time.
*/

#pragma warning(push)
#pragma warning(disable : 4996)

#include "mex.hpp"
#include "mexAdapter.hpp"
#include "boost/numeric/odeint.hpp"
#include <math.h>
#include "CR3BP.h"
#include "EventIntegrator.h"
#include "ExampleEvent.h" // Specify the specific event class to use
#pragma warning(pop)

// Define Namespaces for Convenience
using matlab::mex::ArgumentList; // For Inputs & Outputs to MEX function
using namespace matlab::data;  // For ArrayFactory and MATLAB defined Array types in C++ API
using namespace boost::numeric; // For odeint & steppers

// Define BOOST's 7-8 Stepper as rk78
typedef odeint::runge_kutta_fehlberg78<state_type> rk78;


class MexFunction : public matlab::mex::Function
{
public:

	// What is called from Matlab.  Needs ArgumentLists as inputs
	void operator()(ArgumentList outputs, ArgumentList inputs);

	// Argument Validation function: Throw errors if inputs to MEX function are bad
	void validate_args(ArgumentList outputs, ArgumentList inputs);

	// Function for converting C++ times to MATLAB types
	TypedArray<double> times2matlab(state_type& cpp_times);

	// Function for converting C++ States to MATLAB types
	Array states2matlab(std::vector<state_type>& cpp_states);

	// Function for Converting C++ event indices to MATLAB
	TypedArray<int> events2matlab(std::vector<int>& cpp_ie);
};

/************************************ Function Definitions***************************************************/
// Define MexFunction::operator()
void MexFunction::operator()(ArgumentList outputs, ArgumentList inputs)
{
	// Validate Arguments
	validate_args(outputs, inputs);

	// Stepping Parameters (Default)
	double AbsTol = 1e-16;
	double RelTol = 1e-13;
	double InitStep = 1e-10;

	// Extract Input Info
	TypedArray<double> x0_matlab = (TypedArray<double>) inputs[0];
	TypedArray<double> tspan_matlab = (TypedArray<double>) inputs[1];
	double mu = inputs[2][0]; // Need second index bc of how matlab ArgumentLists work
	int n_cross = (int)inputs[3][0];

	if (inputs.size() == 7) // Stepping parameters specified
	{
		AbsTol = inputs[4][0];
		RelTol = inputs[5][0];
		InitStep = inputs[6][0];
	}

	// Convert States & Times to C++ Types for Use with Boost using Iterators
	state_type x0(x0_matlab.begin(), x0_matlab.end());
	state_type tspan(tspan_matlab.begin(), tspan_matlab.end());

	// Create CR3BP System and Observer
	CR3BP system(mu);

	// Create Event
	XZCross ce(n_cross);

	// Create Event Integrator
	EventIntegrator<rk78> ei(ce, AbsTol, RelTol, InitStep);

	// Integrate
	ei.integrate_adaptive(system, x0, tspan);

	/* Convert States Back to MATLAB Types */
	// Unpack Times
	TypedArray<double> t = times2matlab(ei.integration_results.t);
	TypedArray<double> te = times2matlab(ei.integration_results.te);

	// Unpack States
	Array x = states2matlab(ei.integration_results.x);
	Array xe = states2matlab(ei.integration_results.xe);

	// Unpack Event Indices
	TypedArray<int> ie = events2matlab(ei.integration_results.ie);

	// Set Outputs
	outputs[0] = t;
	outputs[1] = x;
	outputs[2] = te;
	outputs[3] = xe;
	outputs[4] = ie;

}

// Convert vector<double> of times to TypedArray for return to MATLAB
TypedArray<double> MexFunction::times2matlab(state_type& cpp_times)
{
	ArrayFactory af;
	ArrayDimensions dims({ cpp_times.size(), 1 });

	return af.createArray<state_type::iterator>(dims, cpp_times.begin(), cpp_times.end());
}

// Convert vector<double> of states to Array for return to MATLAB
Array MexFunction::states2matlab(std::vector<state_type>& cpp_states)
{
	Array state_matrix;
	ArrayFactory af;
	std::vector<double> linear_storage; // Need linear indexing bc of how MATLAB arrays work

	// Get Linear Contiguous storage in ROW MAJOR order
	for (int i = 0; i < cpp_states.size(); i++)
	{
		for (double j : cpp_states[i])
		{
			linear_storage.push_back(j);
		}
	}

	if (!cpp_states.empty())
	{
		if (cpp_states[0].size() == 6) // Integrated states only
		{
			ArrayDimensions array_dims({ cpp_states.size(), 6 });
			state_matrix = af.createArray<state_type::iterator>(array_dims, linear_storage.begin(), linear_storage.end(),
				InputLayout::ROW_MAJOR);
		}
		else if (cpp_states[0].size() == 42) // Variational equations also
		{
			ArrayDimensions array_dims({ cpp_states.size(), 42 });
			state_matrix = af.createArray<state_type::iterator>(array_dims, linear_storage.begin(), linear_storage.end(),
				InputLayout::ROW_MAJOR);

		}
	}
	else
	{
		state_matrix = af.createEmptyArray();
	}

	return state_matrix;
}


// Convert vector<int> of event indices to Array for return to MATLAB
TypedArray<int> MexFunction::events2matlab(std::vector<int>& cpp_ie)
{
	ArrayFactory af;
	ArrayDimensions dims({ cpp_ie.size(), 1 });

	return af.createArray<std::vector<int>::iterator>(dims, cpp_ie.begin(), cpp_ie.end());
}

// Define MexFunction::validate_args
void MexFunction::validate_args(ArgumentList outputs, ArgumentList inputs)
{
	ArrayFactory af; // For generating matlab::data::Array types
	std::shared_ptr<matlab::engine::MATLABEngine> engine_ptr = getEngine(); // For Throwing Errors if Necessary

	// Validate First Input Arguent (x0, the initial state)
	if (inputs[0].getType() != matlab::data::ArrayType::DOUBLE ||
		(inputs[0].getDimensions() != matlab::data::ArrayDimensions({ 6, 1 }) &&
			inputs[0].getDimensions() != ArrayDimensions({ 42, 1 })))
	{
		engine_ptr->feval(u"error", 0, std::vector<Array>({ af.createScalar(
			"Invalid initial state! Must be 6x1 double column vector!") }));
	}

	// Validate Second Input Argument (t_span, vector of time values)
	if (inputs[1].getType() != matlab::data::ArrayType::DOUBLE ||
		inputs[1].getNumberOfElements() < 2)
	{
		engine_ptr->feval(u"error", 0, std::vector<Array>({ af.createScalar(
			"Invalid time specification!  Must be a double array with at least 2 values!") }));
	}

	// Validate Third Input Argument (mu, mass parameter, scalar)
	if (inputs[2].getType() != matlab::data::ArrayType::DOUBLE ||
		inputs[2].getNumberOfElements() != 1)
	{
		engine_ptr->feval(u"error", 0, std::vector<Array>({ af.createScalar(
			"Invalid Mass Parameter!  Must be a scalar double") }));
	}
}