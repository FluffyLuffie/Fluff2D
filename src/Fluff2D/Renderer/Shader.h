#pragma once

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
	unsigned int ID;

	Shader() {}
	~Shader();

	void setShader(const char* vertexPath, const char* fragmentPath);

	void use();

	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;

	void setMat4(const std::string& name, const glm::mat4& mat) const;
	void setVec4(const std::string& name, const glm::vec4& vec) const;
};