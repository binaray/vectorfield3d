#include "TimeStepper.h"
#include <iostream>

void TimeStepper::takeStep(VectorFieldSystem* particleSystem, float stepSize)
{
	std::vector<Vector3f> u0 = particleSystem->getState();

	Vector3f f0;
	Vector3f f1;
	Vector3f f2;
	Vector3f f3;

	std::vector<Vector3f> u;
	Vector3f u1;
	Vector3f u2;
	Vector3f u3;
	//
	//  Get four sample values of the derivative.
	//


	for (unsigned i = 0; i < particleSystem->getState().size(); i++) {

		f0 = particleSystem->evalF(u0[i]);

		u1 = u0[i] + stepSize * f0 / 2.0;
		f1 = particleSystem->evalF(u1);

		u2 = u0[i] + stepSize * f1 / 2.0;
		f2 = particleSystem->evalF(u2);

		u3 = u0[i] + stepSize * f2;
		f3 = particleSystem->evalF(u3);
		//
		//  Combine to estimate the solution at time T0 + DT.
		//
		u.push_back(u0[i] + stepSize * (f0 + 2.0 * f1 + 2.0 * f2 + f3) / 6.0);
		//TODO: change to update state reference itself
	}

	//particleSystem->setState(u);
}