#pragma once
#include <vector>
#include "VectorFieldSystem.hpp"

class TimeStepper
{
public:
	void takeStep(VectorFieldSystem* particleSystem, float stepSize);
};