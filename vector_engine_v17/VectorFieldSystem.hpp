#pragma once
#include <glad/glad.h>

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.hpp"
#include "PerlinNoise.hpp"

class VectorFieldSystem
{
public:
	const float MAX_MAGNITUDE = 5.0f;
	const float MAX_ARROW_LENGTH = .5f;
	const float SPACING = .5f;
	unsigned int xLength, yLength, zLength;

	//particle simulation states
	std::vector<glm::vec3> m_vVecState;
	unsigned int particleNum;
	float particleInitialVelocity = 0.2f;

	VectorFieldSystem(unsigned xLength, unsigned yLength, unsigned zLength) {
		this->xLength = xLength;
		this->yLength = yLength;
		this->zLength = zLength;

		perlin::initPerlinNoise();

		std::cerr << "Initializing vector field\n"
			<< "xLength: " << this->xLength << std::endl
			<< "yLength: " << this->yLength << std::endl
			<< "zLength: " << this->zLength << std::endl;

		//Vector Field
		for (int i = 0; i < xLength; i++) {
			for (int j = 0; j < yLength; j++) {
				for (int k = 0; k < zLength; k++) {
					//draw position for model matrix
					vecPos.push_back(glm::vec3(i*+SPACING, j*-SPACING, k*-SPACING));
					//vector value to represent
					vecVector.push_back(glm::vec3(0.5f, 0.5f, 0));
					//vector representation positions relative to draw position
					vecDraw.push_back(vecPos.back());
					vecDraw.push_back(-1.0f*vecPos.back());
					//change to sample from particle
					magnitude.push_back(perlin::noise((float)i / (float)xLength, (float)j / (float)yLength, (float)k / (float)zLength));
					//std::cout << magnitude.back() << std::endl;
				}
			}
		}
		
		//Particles --initialized position is before the vector field;
		particleNum = xLength * yLength * zLength;
		for (int i = 0; i < xLength; i++) {
			for (int j = 0; j < yLength; j++) {
				for (int k = 0; k < zLength; k++) {
					m_vVecState.push_back(glm::vec3(i*-0.2, j*-0.2, k*-0.2));
					m_vVecState.push_back(glm::vec3(particleInitialVelocity, 0, 0));
				}
			}
		}

		setupBuffers();
		std::cerr << "Initialized\n";
	}

	std::vector<glm::vec3> evalF(std::vector <glm::vec3> state) {
		//calculate forces from velocity and acceleration
		std::vector <glm::vec3> dX;
		for (int i = 0; i < xLength; i++) {
			for (int j = 0; j < yLength; j++) {
				for (int k = 0; k < zLength; k++) {
					dX.push_back(glm::vec3(0.2f, 0, 0));
					dX.push_back(glm::vec3(0, 0, 0));
				}
			}
		}

		return dX;
	}

	float rotationDegrees = .0f;
	void updateBuffers() {
		glm::vec3 paticleVelocity(4.f, 4.f, 4.f);

		//map to vector4 for computation
		glm::vec4 vectorManip(glm::normalize(paticleVelocity), 1);

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::rotate(transform, rotationDegrees += .1f, glm::vec3(0.0f, 0.0f, 1.0f));

		vectorManip = transform * vectorManip;

		//std::cout << vector[0] << vector[1]<< vector[2] <<std::endl;

		for (unsigned int i = 0; i < vecPos.size(); i++) {
			glm::vec3 vector(vectorManip);
			vector = magnitude[i] / MAX_MAGNITUDE * MAX_ARROW_LENGTH * vector;

			//update start and end draw positions
			vecDraw[i * 2] = vector / 2.0f;
			vecDraw[i * 2 + 1] = vector / 2.0f - vector;

			glBindVertexArray(VAO[i]);
			glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);	// and a different VBO
			glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec3), &vecDraw[i * 2], GL_DYNAMIC_DRAW);

			//single vertex point = (x,y.z)
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
			//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
			glEnableVertexAttribArray(0);
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
			vecShader->setMat4("model", model);
			vecShader->setMat4("transform", glm::mat4(1.0f));
			vecShader->setVec4("vectorColor", magnitude[i], 0.0f, 1.0f - magnitude[i], 1.0f);

			//unsigned int transformLoc = glGetUniformLocation(vecShaders[i]->ID, "transform");
			//glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
			////float greenValue = sin(timeValue) / 2.0f + 0.5f;
			//int vertexColorLocation = glGetUniformLocation(vecShaders[i]->ID, "ourColor");
			//glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f);
			glBindVertexArray(VAO[i]);
			glDrawArrays(GL_LINES, 0, 2);

			//----------------draw particles
			particleShader->use();
			particleShader->setMat4("projection", projection);
			particleShader->setMat4("view", view);
			model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			model = glm::translate(model, m_vVecState[i * 2]);
			particleShader->setMat4("model", model);
			particleShader->setMat4("transform", glm::mat4(1.0f));
			particleShader->setVec4("particleColor", 1.0f, 1.0f, 1.0f, 1.0f);

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
		particleVBO = new unsigned int[vecPos.size()];
		particleVAO = new unsigned int[vecPos.size()];

		glGenVertexArrays(vecPos.size(), particleVAO);
		glGenBuffers(vecPos.size(), particleVBO);
		for (unsigned int i = 0; i < vecPos.size(); i++) {
			glBindVertexArray(particleVAO[i]);
			glBindBuffer(GL_ARRAY_BUFFER, particleVBO[i]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &m_vVecState[i*2], GL_DYNAMIC_DRAW);	//single point only

			//(GLindex, size, type, stride, starting pointer) --stride unused since single point for now
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)0);
			glEnableVertexAttribArray(0);			
		}
	}
};

class VectorParticle {
public:
	glm::vec3 position;	//starting draw point of arrow- does not change
	glm::vec3 vector;
	float magnitude;
	VectorParticle(glm::vec3 position, glm::vec3 vector) : position(position), vector(vector) {}
};