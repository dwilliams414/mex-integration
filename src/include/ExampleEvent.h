#pragma once
#include "Event.h"

/* Example Event: Locate the First n Apses along a trajectory */
class XZCross : public Event
{
public:
	/*	Constructor:
		Write the constructor for your Event class here.  Your specific event (e.g., XZ Cross)
		inherits from the base Event class, which must be instantiated with a vector containing the 
		maximum number of each event to find.  For a single event, you can basically use the 
		constructor template below, where you change n_cross to n_whatever.
	*/
	XZCross(int n_cross) : Event(std::vector<int>(1, n_cross)) {}

	// Event Function: Define your event(s) here, similar to how you would in MATLAB.  Except
	// in this case, items are zero indexed.  Events are defined such that
	// event[i] = 0.  You specify event as a vector
	state_type event_fcn(const double t, const state_type& state) override
	{
		state_type event = { state[1] };
		return event;
	}

	// Terminate Function: Determine if event is terminal
	// 
	std::vector<int> terminate_fcn(const double t, const state_type& state) override
	{
		std::vector<int> terminate = { 0 };

		// This logic is optional, but if it is NOT included, n_cross or the number of events you
		// specified won't really do anything
		if (n_curr_vals[0] >= n_max_vals[0])
		{
			terminate[0] = 1;
		}
		return terminate;
	}

	// Direction Function
	// Specify direction for event finding (same convention as MATLAB)
	std::vector<int> direction_fcn() override
	{
		std::vector<int> direction = { 0 };
		return direction;
	}
};