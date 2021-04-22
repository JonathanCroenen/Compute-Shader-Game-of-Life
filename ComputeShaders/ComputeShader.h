#ifndef COMPUTE_SHADER_H
#define COMPUTE_SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <iostream>

class ComputeShader {
public:
	unsigned int m_ID = NULL;

	ComputeShader() {}

	ComputeShader(const char* path) {
		std::string code;
		std::ifstream shaderFile(path);

		shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			code.assign((std::istreambuf_iterator<char>(shaderFile)),
				(std::istreambuf_iterator<char>()));
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
		}
		const char* shaderCode = code.c_str();

		GLuint compute = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(compute, 1, &shaderCode, NULL);
		glCompileShader(compute);

		checkCompileErrors(compute, "COMPUTE");

		m_ID = glCreateProgram();
		glAttachShader(m_ID, compute);
		glLinkProgram(m_ID);

		checkCompileErrors(m_ID, "PROGRAM");

		glDeleteShader(compute);
	}

	void use() {
		glUseProgram(m_ID);
	}

	void dispatch(GLuint groupsX, GLuint groupsY, GLuint groupsZ) {
		glDispatchCompute(groupsX, groupsY, groupsZ);
	}

	void setBool(std::string name, bool value) const {
		glUniform1i(glGetUniformLocation(m_ID, name.c_str()), (int)value);
	}
	void setInt(std::string name, int value) const {
		glUniform1i(glGetUniformLocation(m_ID, name.c_str()), value);
	}
	void setFloat(std::string name, float value) const {
		glUniform1f(glGetUniformLocation(m_ID, name.c_str()), value);
	}

	void setVec2(std::string name, glm::vec2 value) const {
		glUniform2fv(glGetUniformLocation(m_ID, name.c_str()), 1, &(value.x));
	}
	void setVec2(std::string name, float x, float y) const {
		glUniform2f(glGetUniformLocation(m_ID, name.c_str()), x, y);
	}

	void setVec3(std::string name, glm::vec3 value) const {
		glUniform3fv(glGetUniformLocation(m_ID, name.c_str()), 1, &(value.x));
	}
	void setVec3(std::string name, float x, float y, float z) const {
		glUniform3f(glGetUniformLocation(m_ID, name.c_str()), x, y, z);
	}

	void setVec4(std::string name, glm::vec4 value) const {
		glUniform4fv(glGetUniformLocation(m_ID, name.c_str()), 1, &(value.x));
	}
	void setVec4(std::string name, float x, float y, float z, float w) const {
		glUniform4f(glGetUniformLocation(m_ID, name.c_str()), x, y, z, w);
	}

	void setMat2(std::string name, glm::mat2 value) const {
		glUniformMatrix2fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, &(value[0].x));
	}
	void setMat3(std::string name, glm::mat3 value) const {
		glUniformMatrix3fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, &(value[0].x));
	}
	void setMat4(std::string name, glm::mat4 value) const {
		glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, &(value[0].x));
	}

private:
	void checkCompileErrors(GLuint shader, std::string type) {
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM") {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::" << type << "::SHADER::COMPILATION_ERROR" << "\n" << infoLog << "\n --------------------------------------------------------- " << std::endl;
			}
		}
		else {
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER::" << type << "::LINKING_ERROR" << "\n" << infoLog << "\n --------------------------------------------------------- " << std::endl;
			}
		}
	}
};

#endif //COMPUTE_SHADER_H
