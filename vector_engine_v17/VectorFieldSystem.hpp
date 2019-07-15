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
	unsigned int xLength, yLength, zLength;

	//particle simulation states
	std::vector<glm::vec3> m_vVecState;

	VectorFieldSystem(unsigned xLength, unsigned yLength, unsigned zLength) {
		this->xLength = xLength;
		this->yLength = yLength;
		this->zLength = zLength;

		std::cerr << "Initializing vector field\n"
			<< "xLength: " << this->xLength << std::endl
			<< "yLength: " << this->yLength << std::endl
			<< "zLength: " << this->zLength << std::endl;

		//Vector Field
		for (int i = 0; i < xLength; i++) {
			for (int j = 0; j < yLength; j++) {
				for (int k = 0; k < zLength; k++) {
					vecPos.push_back(glm::vec3(i*+0.2, j*-0.2, k*-0.2));
					vecVector.push_back(glm::vec3(0.5f, 0.5f, 0));
					vecDraw.push_back(vecPos.back());
					vecDraw.push_back(-1.0f*vecPos.back());
					magnitude.push_back(1.0f);
				}
			}
		}
		
		//Particles
		for (int i = 0; i < xLength; i++) {
			for (int j = 0; j < yLength; j++) {
				for (int k = 0; k < zLength; k++) {
					m_vVecState.push_back(glm::vec3(i*+0.2, j*-0.2, k*-0.2));
					m_vVecState.push_back(glm::vec3(0, 0, 0));
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
					dX.push_back(glm::vec3(-.2f, 0, 0));
					dX.push_back(glm::vec3(0, 0, 0));
				}
			}
		}

		return dX;
	}

	float rotationDegrees = .0f;
	void updateBuffers() {
		glm::vec4 vectorManip(2.5f, 2.5f, 2.5f, 1);
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::rotate(transform, rotationDegrees += .1f, glm::vec3(0.0f, 0.0f, 1.0f));
		vectorManip = transform * vectorManip;

		glm::vec3 vector(vectorManip);
		//std::cout << vector[0] << vector[1]<< vector[2] <<std::endl;

		for (unsigned int i = 0; i < vecPos.size(); i++) {

			//update start and end draw positions
			vecDraw[i * 2] = vector / 2.0f;
			vecDraw[i * 2 + 1] = vector / 2.0f - vector;
			//std::cout << vecDraw[i * 2][0] << vecDraw[i * 2][1] << vecDraw[i * 2][2] << std::endl;
			//std::cout << vecDraw[i * 2 + 1][0] << vecDraw[i * 2 + 1][1] << vecDraw[i * 2 + 1][2] << std::endl;

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
			vecShaders[i]->use();
			vecShaders[i]->setMat4("projection", projection);
			vecShaders[i]->setMat4("view", view);

			glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			model = glm::translate(model, vecPos[i]);
			vecShaders[i]->setMat4("model", model);
			vecShaders[i]->setMat4("transform", glm::mat4(1.0f));
			vecShaders[i]->setVec4("ourColor", 1.0f, 0.0f, 0.0f, 1.0f);

			//unsigned int transformLoc = glGetUniformLocation(vecShaders[i]->ID, "transform");
			//glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

			////float greenValue = sin(timeValue) / 2.0f + 0.5f;
			//int vertexColorLocation = glGetUniformLocation(vecShaders[i]->ID, "ourColor");
			//glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f);

			glBindVertexArray(VAO[i]);
			glDrawArrays(GL_LINES, 0, 2);
		}
	}

private:
	//vector representations
	std::vector<glm::vec3> vecPos;
	std::vector<glm::vec3> vecVector;	//not necessary if checking from particles
	std::vector<glm::vec3> vecDraw;	//startPos	endPos	vector
	std::vector<float> magnitude;


	/*  Render data  */
	unsigned int *VBO;
	unsigned int *VAO;
	typedef Shader* ShaderPtr;
	ShaderPtr *vecShaders;

	void setupBuffers() {
		// build and compile our shader program
		std::cerr << "Initializing shaders\n";
		vecShaders = new ShaderPtr[vecPos.size()];
		
		std::cerr << "Initializing vector field buffer objects\n";
		VBO = new unsigned int[vecPos.size()];
		VAO = new unsigned int[vecPos.size()];
		
		std::cout << vecPos.size();
		glGenVertexArrays(vecPos.size(), VAO);
		glGenBuffers(vecPos.size(), VBO);
		for (unsigned int i = 0; i < vecPos.size(); i++) {
			glBindVertexArray(VAO[i]);
			glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);	// and a different VBO
			glBufferData(GL_ARRAY_BUFFER, 2*sizeof(glm::vec3), &vecDraw[i*2], GL_DYNAMIC_DRAW);

			//single vector point so 1 vertex for now
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0); // because the vertex data is tightly packed we can also specify 0 as the vertex attribute's stride to let OpenGL figure it out
			//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
			glEnableVertexAttribArray(0);

			vecShaders[i] = new Shader("shaders/vectorfield.vs", "shaders/vectorfield.fs");
		}

		//std::cerr << "Initializing particle buffer objects\n";
		//VBO = new unsigned int[vecPos.size()];
		//VAO = new unsigned int[vecPos.size()];

		//for (unsigned int i = 0; i < vecPos.size(); i++) {
		//	glGenVertexArrays(1, &VAO[i]);
		//	glGenBuffers(1, &VBO[i]);
		//	glBindVertexArray(VAO[i]);
		//	glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);	// and a different VBO
		//	glBufferData(GL_ARRAY_BUFFER, m_vVecState.size() / 2 * sizeof(glm::vec3), &m_vVecState[0], GL_DYNAMIC_DRAW);

		//	//single vector point so 1 vertex
		//	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*)0); // because the vertex data is tightly packed we can also specify 0 as the vertex attribute's stride to let OpenGL figure it out
		//	glEnableVertexAttribArray(0);
		//}
	}
};

class VectorParticle {
public:
	glm::vec3 position;	//starting draw point of arrow- does not change
	glm::vec3 vector;
	float magnitude;
	VectorParticle(glm::vec3 position, glm::vec3 vector) : position(position), vector(vector) {}
};