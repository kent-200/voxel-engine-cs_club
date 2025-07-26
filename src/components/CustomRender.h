#pragma once
#include <iostream>
#include <vector>
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/shader_m.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include "Texture.h"




/*
Render
has all opengl rendering functions, manages loading shaders, images and rendering 
instance of this class is created in main.cpp
*/


class Render {
private:
	const int SHADER_INPUT_SIZE = 7;	// x, y, z, r, g, b, shadow	// number of floats per vertex passed as layout

    // file paths
    std::string vertexShaderPath = "src/shaders/shader.vert";
    std::string fragmentShaderPath = "src/shaders/shader.frag";

	std::string texturePath = "src/textures/terrain.png";

    GLuint shaderProgram;
    GLuint VAO, VBO, EBO;
	GLuint texture;

    glm::mat4 projectionMatrix;

    
	void shaderInit(){
		// 1. load the shader files
		std::string vertexShaderCode;
		std::string fragmentShaderCode;

		try {
			std::ifstream vertexShaderStream(vertexShaderPath, std::ios::in);
			std::ifstream fragmentShaderStream(fragmentShaderPath, std::ios::in);

			if(!vertexShaderStream.is_open()) throw std::runtime_error("Cannot open vertex shader file");
			if(!fragmentShaderStream.is_open()) throw std::runtime_error("Cannot open fragment shader file");

			std::stringstream sstr;
			sstr << vertexShaderStream.rdbuf();
			vertexShaderCode = sstr.str();
			vertexShaderStream.close();

			sstr.str("");
			sstr << fragmentShaderStream.rdbuf();
			fragmentShaderCode = sstr.str();
			fragmentShaderStream.close();
			
		} catch (std::exception e){
			std::cerr << "Error: " << e.what() << std::endl;
		}

		if (vertexShaderCode.empty() || fragmentShaderCode.empty()) {
			std::cerr << "Error: Shader code is empty." << std::endl;
			return;
		}
		

		const char * vertexShaderPtr = vertexShaderCode.c_str();
		const char * fragmentShaderPtr = fragmentShaderCode.c_str();


		// 2. compile the shaders
		int success;
		// vertex
		GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vertexShaderPtr, NULL);
		glCompileShader(vertex);
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cerr << "Error: Vertex Shader Compilation Failed\n" << infoLog << std::endl;
		}



		// fragment
		GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fragmentShaderPtr, NULL);
		glCompileShader(fragment);
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cerr << "Error: Fragment Shader Compilation Failed\n" << infoLog << std::endl;
		}

		// create shader program
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertex);
		glAttachShader(shaderProgram, fragment);
		glLinkProgram(shaderProgram);
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);	// check sucess
		if(!success){
			char infoLog[512];
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cerr << "Error: Shader Program Linking Failed\n" << infoLog << std::endl;
		}

		// delete shaders - now linked to program, no longer needed
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}


	void createBuffers(){

		// Create Vertex Array Object
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		// Create a Vertex Buffer Object and copy the vertex data to it
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW); // will be updated later

		// create an Element Buffer Object
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);	// will be updated later


		// define the vertex attribute pointer
		// for positions - layer 0
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, SHADER_INPUT_SIZE * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		// for colors - layer 1
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, SHADER_INPUT_SIZE * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		// for shadows - layer 2, 1 number
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, SHADER_INPUT_SIZE * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);

		// Unbind the VAO
		glBindVertexArray(0);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

	}


	void loadTexture(){
		int width, height, nrChannels;
		unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
		if (data){
			glGenTextures(1, &texture);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);

			// Set the texture wrapping parameters, chose pixelated look
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			// repeated setting - want to stretch on both x any y axis
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


			// generate texture
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			stbi_image_free(data);
		} else {
			std::cerr << "Error: Failed to load texture" << std::endl;
			exit(0);
		}

		GLuint textureLoc = glGetUniformLocation(shaderProgram, "tex0");
		glUseProgram(shaderProgram);
		glUniform1i(textureLoc, 0);

	}

public:
    Render(){}


	bool init(int windowWidth, int windowHeight){
		projectionMatrix = glm::perspective(glm::radians(90.0f), (float) windowWidth / (float) windowHeight, 0.1f, 1000.0f);
		shaderInit();
        createBuffers();
		loadTexture();

		glUseProgram(shaderProgram);
		// set projection matrix in shader
		GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		return true;
	}


    // Render function, called to render the object 
    // format{ x y z r g b,} for 3 points, per triangle
    bool renderData(glm::mat4 viewMatrix, float * verticies, int verticies_count, unsigned int * indicies, int indicies_count, bool transparent = false){
        if(verticies_count == 0 || indicies_count == 0){
           // std::cout << "Vertex Data is empty" << std::endl;
            return false;
        }

		// defined in shader
		//glm::mat4 worldMatrix = glm::mat4(1.0f);	// Form World Matrix

		if(transparent){
			glDisable(GL_CULL_FACE);	
			glEnable(GL_BLEND);       // Enable blending for transparent objects
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Set blending function
		} else {
			glDisable(GL_BLEND);      // Disable blending for opaque objects
			glEnable(GL_CULL_FACE);   // Enable backface culling
			glCullFace(GL_BACK);      // Cull back faces
			glFrontFace(GL_CCW);      
		}
		

		// Use the shader program
		glUseProgram(shaderProgram);


		// bind texture
		glBindTexture(GL_TEXTURE_2D, texture);


		// Pass matrices to the shader
		//GLint modelLoc = glGetUniformLocation(shaderProgram, "model");	// define in shader for performace
		GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
		// GLint projLoc = glGetUniformLocation(shaderProgram, "projection");	// passed at start
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		// glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));


		// 5. Update Vertex Buffer Object (VBO) - new data
		// use method: glBufferSubData not glMapBuffer
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, verticies_count * sizeof(float), verticies, GL_DYNAMIC_DRAW);


		// 6. Update Element Buffer Object (EBO) - new data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies_count * sizeof(unsigned int), indicies, GL_DYNAMIC_DRAW);


		// 7. Render the object
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glDrawElements(GL_TRIANGLES, indicies_count, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		return true;
		
	}



	// Destructor
	void destroy(){
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		glDeleteProgram(shaderProgram);

		// Clean up and exit
    	glfwTerminate();
	}
};