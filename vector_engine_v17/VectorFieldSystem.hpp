#pragma once
#include <glad/glad.h>

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "tuple.h"

#include "Shader.hpp"
#include "PerlinNoise.hpp"

class VectorFieldSystem
{
public:
	const float MAX_MAGNITUDE = 5.0f;
	const float MAX_ARROW_LENGTH = .25f;
	const float SPACING = 1.f;
	const float PARTICLE_OFFSET = .5f;
	unsigned int xLength, yLength, zLength;

	//particle simulation states
	std::vector<glm::vec3> m_vVecState;
	std::vector<unsigned int> ttl;

	unsigned int particleNum;
	float particleInitialVelocity = 0.2f;

	//-----game vars
	typedef tuple< unsigned, 3 > Tuple3u;
	int difficulty = -1;
	bool isVictorious = false;
	int startIndex = 0;	//x,y,z index with respect to field system
	int endIndex = 0;

	std::vector<int> manipType;	//0- rotates on xy plane; 1- rotates on xz plane; 2-rotates on yz plane 
	std::vector<int> manipIndex;

	VectorFieldSystem(unsigned xLength, unsigned yLength, unsigned zLength) {
		this->xLength = xLength;
		this->yLength = yLength;
		this->zLength = zLength;

		perlin::initPerlinNoise(241);

		std::cerr << "Initializing vector field\n"
			<< "xLength: " << this->xLength << std::endl
			<< "yLength: " << this->yLength << std::endl
			<< "zLength: " << this->zLength << std::endl;

		//---------------Vector Field-----------------
		for (int i = 0; i < xLength; i++) {
			for (int j = 0; j < yLength; j++) {
				for (int k = 0; k < zLength; k++) {
					glm::vec3 vector = glm::normalize(glm::vec3(0.5f,0,0));	//default unit vector
					float mag = 2.0f * perlin::noise((float)i / (float)xLength, (float)j / (float)yLength, (float)k / (float)zLength);	//default magnitude

					if (i > xLength/2) {
						vector = glm::normalize(glm::vec3(-1.f, 0, 0));
						mag = 4.f;
					}


					//store draw position for model matrix; unit vector value; magnitude respectively
					vecPos.push_back(glm::vec3((i * +SPACING) + SPACING / 2.0f, (j * -SPACING) - SPACING / 2.0f, (k * -SPACING) - SPACING / 2.0f));
					vecVector.push_back(vector);
					magnitude.push_back(mag);

					//change to reflect draw size
					vector = mag / MAX_MAGNITUDE * MAX_ARROW_LENGTH * vector;
					//vector representation positions relative to draw position
					vecDraw.push_back(vector / 2.0f);
					vecDraw.push_back(vector / 2.0f - vector);
				}
			}
		}
		
		//-----------Particles --initialized position is before the vector field--------
		particleNum = xLength * yLength * zLength;
		for (int i = 0; i < xLength; i++) {
			for (int j = 0; j < yLength; j++) {
				for (int k = 0; k < zLength; k++) {
					m_vVecState.push_back(glm::vec3(i*-PARTICLE_OFFSET, -0.1 + j*-PARTICLE_OFFSET, -0.1 + k*-PARTICLE_OFFSET));
					m_vVecState.push_back(glm::vec3(particleInitialVelocity, 0, 0));
				}
			}
		}

		setupBuffers();
		std::cerr << "Initialized\n";
	}

	unsigned int dimen3ToIndex(int x, int y, int z) {

		return x * yLength * zLength + y * zLength + z;
	}

	VectorFieldSystem(int difficulty) {
		this->difficulty = difficulty;
		switch (difficulty)
		{
		case 1:
		case 0:
		default:
			perlin::initPerlinNoise(241);
			xLength = 5;
			yLength = 5;
			zLength = 5;
			startIndex = dimen3ToIndex(0, 0, 0);
			endIndex = dimen3ToIndex(4, 0, 0);

			manipType.push_back(0);
			manipIndex.push_back(dimen3ToIndex(0, 0, 0));
			manipType.push_back(1);
			manipIndex.push_back(dimen3ToIndex(3, 0, 0));
			break;
		}
		std::cout << "Starting stage " << difficulty << std::endl;

		//init noisy field first
		for (int i = 0; i < xLength; i++) {
			for (int j = 0; j < yLength; j++) {
				for (int k = 0; k < zLength; k++) {
					glm::vec3 vector = glm::normalize(glm::vec3(0.5f, 0.f, 0));	//default unit vector
					float mag = perlin::noise((float)i / (float)xLength, (float)j / (float)yLength, (float)k / (float)zLength);	//default magnitude

					//store draw position for model matrix; unit vector value; magnitude respectively
					vecPos.push_back(glm::vec3((i * +SPACING) + SPACING / 2.0f, (j * -SPACING) - SPACING / 2.0f, (k * -SPACING) - SPACING / 2.0f));
					vecVector.push_back(vector);
					magnitude.push_back(mag);

					//change to reflect draw size
					vector = mag / MAX_MAGNITUDE * MAX_ARROW_LENGTH * vector;
					//vector representation positions relative to draw position
					vecDraw.push_back(vector / 2.0f);
					vecDraw.push_back(vector / 2.0f - vector);
				}
			}
		}

		//Overwrite field with game values
		float mag = 5.0f;
		for (int i = 0; i < manipType.size(); i++) {
			int index = manipIndex[i];
			glm::vec3 vector = glm::normalize(glm::vec3(0.5f, 0, 0));
			if (manipType[i] == 0) {
				vector = glm::normalize(glm::vec3(0, 1.f, 0));
			}
			else if (manipType[i] == 1) {
				vector = glm::normalize(glm::vec3(1.f, 0, 0));
			}
			else {
				vector = glm::normalize(glm::vec3(0, 0, 1.f));
			}
			vecVector[index] = vector;
			magnitude[index] = mag;

			vector = mag / MAX_MAGNITUDE * MAX_ARROW_LENGTH * vector;
			vecDraw[index] = vector / 2.0f;
			vecDraw[index+1] = vector / 2.0f - vector;
		}

		particleNum = 1;
		m_vVecState.push_back(vecPos[startIndex]);
		m_vVecState.push_back(glm::vec3(particleInitialVelocity, 0, 0));

		setupBuffers();
	}


	//---------------------------------check and compute effector result on particles----------------------------
	std::vector<glm::vec3> evalF(std::vector <glm::vec3> state) {
		//calculate forces from velocity and acceleration
		std::vector <glm::vec3> dX;
		//std::cout << m_vVecState[24].x<<std::endl;
		for (int i = 0; i < particleNum; i++) {
			glm::vec3 acceleration = glm::vec3(.0f, .0f, .0f);
			
			//Vector field to particle interaction
			//reset condition
			if (m_vVecState[i * 2].x < -PARTICLE_OFFSET * xLength ||
				m_vVecState[i * 2].x >= float(xLength * SPACING) || 
				m_vVecState[i * 2].y <= float((yLength-1) * -SPACING) || 
				m_vVecState[i * 2].y >= .0f || 
				m_vVecState[i * 2].z <= float((zLength-1) * -SPACING) || 
				m_vVecState[i * 2].z >= 0.0f) 
			{
				int jk = i % (zLength * yLength);	//cross section
				int k = jk % yLength;
				int j = jk / zLength;
				m_vVecState[i * 2 + 1] = glm::vec3(particleInitialVelocity, 0, 0);
				m_vVecState[i * 2] = glm::vec3(0.0f, -0.1 + j * -PARTICLE_OFFSET, -0.1 + k * -PARTICLE_OFFSET);
				acceleration = glm::vec3(.0f, .0f, .0f);
			}
			else if (m_vVecState[i * 2].x > 0.0f) {
				//haxx check
				//since x, y, z is bounded, we can safely convert coordinates to vector field memory space
				int x = m_vVecState[i * 2].x / SPACING;
				int y = -m_vVecState[i * 2].y / SPACING;	//positive space of y and z not used
				int z = -m_vVecState[i * 2].z / SPACING;

				
				unsigned int index = x * yLength * zLength + y * zLength + z;

				if (difficulty > -1 && index == endIndex) isVictorious = true;
				/*
				std::cout << x << std::endl;
				std::cout << index << std::endl;*/
				
				acceleration = magnitude[index] * vecVector[index];
				//std::cout << acceleration.x << std::endl;
			}
			dX.push_back(m_vVecState[i * 2 + 1]);
			dX.push_back(acceleration);
		}

		return dX;
	}

	float rotationDegrees = 1.0f;
	//----------------------------------------for game use only---------------------------------------------
	void updateBuffers() {
		////particle to vector interaction
		glm::vec3 paticleVelocity(4.f, 0, 0);

		//for (unsigned int i = 0; i < vecPos.size(); i++) {
		//	glm::vec4 vectorManip(glm::normalize(paticleVelocity), 1);
		//	glm::mat4 transform = glm::mat4(1.0f);
		//	transform = glm::rotate(transform, rotationDegrees = i*1.8f, glm::vec3(0.0f, 0.0f, 1.0f));

		//	vectorManip = transform * vectorManip;

		//	glm::vec3 vector(vectorManip);
		//	vector = magnitude[i] / MAX_MAGNITUDE * MAX_ARROW_LENGTH * vector;
		//	vecVector[i] = vector;

		//	//update start and end draw positions
		//	vecDraw[i * 2] = vector / 2.0f;
		//	vecDraw[i * 2 + 1] = vector / 2.0f - vector;

		//	glBindVertexArray(VAO[i]);
		//	glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);	// and a different VBO
		//	//glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec3), &vecDraw[i * 2], GL_DYNAMIC_DRAW);
		//	glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * sizeof(glm::vec3), &vecDraw[i * 2]);

		//	//OpenGL3.3++: VAOs store vertex attributes so redeclaration is not needed
		//	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		//	//glEnableVertexAttribArray(0);

		//}

		//---this rotates the end flag to show goal only for now
		if (difficulty > -1) {
			glm::vec4 vectorManip(glm::normalize(paticleVelocity), 1);
			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::rotate(transform, rotationDegrees += .8f, glm::vec3(0.0f, 0.0f, 1.0f));

			vectorManip = transform * vectorManip;

			glm::vec3 vector(vectorManip);
			vector = MAX_ARROW_LENGTH * vector;
			vecVector[endIndex] = vector;

			//update start and end draw positions
			vecDraw[endIndex * 2] = vector / 2.0f;
			vecDraw[endIndex * 2 + 1] = vector / 2.0f - vector;

			glBindVertexArray(VAO[endIndex]);
			glBindBuffer(GL_ARRAY_BUFFER, VBO[endIndex]);	// and a different VBO
			//glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec3), &vecDraw[i * 2], GL_DYNAMIC_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * sizeof(glm::vec3), &vecDraw[endIndex * 2]);
		}
	}

	void rotateVector(int key) {
		if (key < manipType.size()) {
			//Overwrite field with game values
			float mag = 5.0f;
			int index = manipIndex[key];

			glm::vec4 vectorManip(vecVector[index], 1);
			glm::mat4 transform = glm::mat4(1.0f);

			if (manipType[key] == 0) {
				transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			}
			else if (manipType[key] == 1) {
				transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			}
			else {
				transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			}
			vectorManip = transform * vectorManip;
			glm::vec3 vector(vectorManip);

			vecVector[index] = vector;
			magnitude[index] = mag;

			vector = mag / MAX_MAGNITUDE * MAX_ARROW_LENGTH * vector;
			vecDraw[index*2] = vector / 2.0f;
			vecDraw[index*2 + 1] = vector / 2.0f - vector;

			glBindVertexArray(VAO[index]);
			glBindBuffer(GL_ARRAY_BUFFER, VBO[index]);	// and a different VBO
			glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * sizeof(glm::vec3), &vecDraw[index*2]);
		}
	}

	void draw(glm::mat4 &projection, glm::mat4 &view) {
		//std::cerr << "Drawing vector field\n";

		// create transformations
		//glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		//transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));
		//transform = glm::scale(transform, glm::vec3(0.0f, 0.0f, 1.0f));
		//transform = glm::rotate(transform, (float)5, glm::vec3(0.0f, 0.0f, 1.0f));
		
		for (unsigned int i = 0; i < vecPos.size(); i++) {
			
			// get matrix's uniform location and set matrix
			vecShader->use();
			vecShader->setMat4("projection", projection);
			vecShader->setMat4("view", view);

			glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			model = glm::translate(model, vecPos[i]);
			vecShader->setMat4("model", model);	//TODO: change to attribute since permanent
			vecShader->setMat4("transform", glm::mat4(1.0f));
			vecShader->setVec4("vectorColor", magnitude[i], 0.0f, 1.0f - magnitude[i], 1.0f);

			//unsigned int transformLoc = glGetUniformLocation(vecShaders[i]->ID, "transform");
			//glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
			////float greenValue = sin(timeValue) / 2.0f + 0.5f;
			//int vertexColorLocation = glGetUniformLocation(vecShaders[i]->ID, "ourColor");
			//glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f);
			glBindVertexArray(VAO[i]);
			glDrawArrays(GL_LINES, 0, 2);

		}

		for (int i = 0; i < particleNum; i++) {

			//----------------draw particles
			particleShader->use();
			particleShader->setMat4("projection", projection);
			particleShader->setMat4("view", view);
			glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			model = glm::translate(model, m_vVecState[i * 2]);
			particleShader->setMat4("model", model);
			particleShader->setMat4("transform", glm::mat4(1.0f));
			particleShader->setVec4("particleColor", 0.7f, 0.0f, 0.0f, 1.0f);

			glBindVertexArray(particleVAO[i]);
			glDrawArrays(GL_POINTS, 0, 1);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particleVBO[i]);
			//glDrawElements(GL_POINT, 3, GL_FLOAT, (void*)0);
		}
	}

private:
	//vector representations
	std::vector<glm::vec3> vecPos;
	std::vector<glm::vec3> vecVector;	//not necessary if checking from particles
	std::vector<glm::vec3> vecDraw;	//startPos	endPos	vector
	std::vector<float> magnitude;


	/*  Vector field render data  */
	unsigned int *VBO;
	unsigned int *VAO;
	//typedef Shader* ShaderPtr;
	//ShaderPtr *vecShaders;
	Shader *vecShader;

	/*  Particle render data  */
	unsigned int *particleVBO;
	unsigned int *particleVAO;
	Shader *particleShader;

	void setupBuffers() {
		std::cerr << "Initializing shaders\n";
		//vecShaders = new ShaderPtr[vecPos.size()];
		vecShader = new Shader("shaders/vectorfield.vs", "shaders/vectorfield.fs");
		particleShader = new Shader("shaders/particle.vs", "shaders/particle.fs");

		std::cerr << "Initializing vector field buffer objects\n";
		VBO = new unsigned int[vecPos.size()];
		VAO = new unsigned int[vecPos.size()];
		
		glGenVertexArrays(vecPos.size(), VAO);
		glGenBuffers(vecPos.size(), VBO);
		for (unsigned int i = 0; i < vecPos.size(); i++) {
			glBindVertexArray(VAO[i]);
			glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
			glBufferData(GL_ARRAY_BUFFER, 2*sizeof(glm::vec3), &vecDraw[i*2], GL_DYNAMIC_DRAW);	//2 points

			//(GLindex, size, type, stride, starting pointer)
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // because the vertex data is tightly packed we can also specify 0 as the vertex attribute's stride to let OpenGL figure it out
			//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
			glEnableVertexAttribArray(0);

		}

		//std::cerr << "Initializing particle buffer objects\n";
		particleVBO = new unsigned int[particleNum];
		particleVAO = new unsigned int[particleNum];

		glGenVertexArrays(particleNum, particleVAO);
		glGenBuffers(particleNum, particleVBO);
		for (unsigned int i = 0; i < particleNum; i++) {
			glBindVertexArray(particleVAO[i]);
			glBindBuffer(GL_ARRAY_BUFFER, particleVBO[i]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &m_vVecState[i*2], GL_DYNAMIC_DRAW);	//single point only

			//(GLindex, size, type, stride, starting pointer) --stride unused since single point for now
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)0);
			glEnableVertexAttribArray(0);			
		}
	}
};