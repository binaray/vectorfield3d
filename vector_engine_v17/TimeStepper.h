#pragma once
#include "vecmath/vecmath.h"
#include <vector>
#include "VectorFieldSystem.hpp"

class TimeStepper
{
	void takeStep(VectorFieldSystem* particleSystem, float stepSize);
};