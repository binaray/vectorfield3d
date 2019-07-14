#pragma once
#include <glad/glad.h>

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "PerlinNoise.hpp"

class VectorFieldSystem
{
public:
	unsigned int xLength, yLength, zLength;

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
					vecVector.push_back(glm::vec3(0, 0, 0));
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

	std::vector<glm::vec3> getState() {
		return m_vVecState;
	}

	void setState(std::vector<glm::vec3> newState) {
		m_vVecState = newState;
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

	void draw() {
		//std::cerr << "Drawing vector field\n";
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, m_vVecState.size()/2);
	}

private:
	//vector representations
	std::vector<glm::vec3> vecPos;
	std::vector<glm::vec3> vecVector;

	//particle simulations
	std::vector<glm::vec3> m_vVecState;

	// shaders
	const char *vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\0";
	const char *fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{\n"
		"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\n\0";

	/*  Render data  */
	unsigned int VBO, VAO;
	unsigned int shaderProgram;

	void setupBuffers() {
		// build and compile our shader program
		std::cerr << "Initializing shaders\n";
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		unsigned int fragmentShaderOrange = glCreateShader(GL_FRAGMENT_SHADER); // the first fragment shader that outputs the color orange
		shaderProgram = glCreateProgram();
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		glShaderSource(fragmentShaderOrange, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShaderOrange);
		// link the first program object
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShaderOrange);
		glLinkProgram(shaderProgram);
		
		std::cerr << "Initializing buffer objects\n";
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);	// note that we bind to a different VAO now
		glBindBuffer(GL_ARRAY_BUFFER, VBO);	// and a different VBO
		glBufferData(GL_ARRAY_BUFFER, m_vVecState.size() * sizeof(glm::vec3), &m_vVecState[0], GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // because the vertex data is tightly packed we can also specify 0 as the vertex attribute's stride to let OpenGL figure it out
		glEnableVertexAttribArray(0);
	}
};

class VectorParticle {
public:
	glm::vec3 position;	//starting draw point of arrow- does not change
	glm::vec3 vector;
	float magnitude;
	VectorParticle(glm::vec3 position, glm::vec3 vector) : position(position), vector(vector) {}
};