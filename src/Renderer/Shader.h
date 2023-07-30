#pragma once

#include <unordered_map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>

class Shader
{
public:
	unsigned int ID = 0;

	Shader() {}
	~Shader() { glDeleteProgram(ID); }

	void setShader(const char* vertexPath, const char* fragmentPath);

	void use();

	void setBool(const std::string& name, bool value);
	void setInt(const std::string& name, int value);
	void setFloat(const std::string& name, float value);

	void setMat4(const std::string& name, const glm::mat4& mat);
	void setVec3(const std::string& name, const glm::vec3& vec);
	void setVec4(const std::string& name, const glm::vec4& vec);
private:
	std::unordered_map<std::string, GLint> uniformLocations;
	GLint getUniformLocation(const std::string& name);
};