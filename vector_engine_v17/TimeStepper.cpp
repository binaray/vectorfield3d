#include "TimeStepper.h"
#include <iostream>

void TimeStepper::takeStep(VectorFieldSystem* particleSystem, float stepSize)
{
	std::vector<glm::vec3> &u0 = particleSystem->m_vVecState;

	std::vector<glm::vec3> f0;
	std::vector<glm::vec3> f1;
	std::vector<glm::vec3> f2;
	std::vector<glm::vec3> f3;

	std::vector<glm::vec3> u;
	std::vector<glm::vec3> u1;
	std::vector<glm::vec3> u2;
	std::vector<glm::vec3> u3;
	//
	//  Get four sample values of the derivative.
	//
	f0 = particleSystem->evalF(u0);


	for (unsigned i = 0; i < u0.size(); i++) {
		u1.push_back(u0[i] + stepSize * f0[i] / 2.0f);
	}
	f1 = particleSystem->evalF(u1);

	for (unsigned i = 0; i < u0.size(); i++) {
		u2.push_back(u0[i] + stepSize * f1[i] / 2.0f);
	}
	f2 = particleSystem->evalF(u2);

	for (unsigned i = 0; i < u0.size(); i++) {
		u3.push_back(u0[i] + stepSize * f2[i]);
	}
	f3 = particleSystem->evalF(u3);

	for (unsigned i = 0; i < u0.size(); i++) {
		//  Combine to estimate the solution at time T0 + DT.
		u.push_back(u0[i] + stepSize * (f0[i] + 2.0f * f1[i] + 2.0f * f2[i] + f3[i]) / 6.0f);
	}
	//std::cout << "u[0].x is now: "<< u[0][0] << std::endl;
	particleSystem->m_vVecState = u;
}

//{
//	std::vector<Vector3f> u0 = particleSystem->getState();
//
//	Vector3f f0;
//	Vector3f f1;
//	Vector3f f2;
//	Vector3f f3;
//
//	std::vector<Vector3f> u;
//	Vector3f u1;
//	Vector3f u2;
//	Vector3f u3;
//	//
//	//  Get four sample values of the derivative.
//	//
//
//
//	for (unsigned i = 0; i < particleSystem->getState().size(); i++) {
//
//		f0 = particleSystem->evalF(u0[i]);
//
//		u1 = u0[i] + stepSize * f0 / 2.0;
//		f1 = particleSystem->evalF(u1);
//
//		u2 = u0[i] + stepSize * f1 / 2.0;
//		f2 = particleSystem->evalF(u2);
//
//		u3 = u0[i] + stepSize * f2;
//		f3 = particleSystem->evalF(u3);
//		//
//		//  Combine to estimate the solution at time T0 + DT.
//		//
//		u.push_back(u0[i] + stepSize * (f0 + 2.0 * f1 + 2.0 * f2 + f3) / 6.0);
//		//TODO: change to update state reference itself
//	}
//
//	//particleSystem->setState(u);
//}