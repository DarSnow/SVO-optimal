#pragma once
//shader compile and link
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Common/GLheader.h"

namespace ModelClass {
	inline std::string SourceFromFile(const std::string &filepath)
	{
		std::ifstream file(filepath);
		std::string result((std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>());
		file.close();
		return result;
	}

	class Shader
	{
	public:
		enum shaderType { vertexShader, fragmentShader, geometryShader, computeShader };
		// cannot put this to private , because map need this to return an empty shader
		Shader() { }
		// Constructor generates the shader on the fly
		Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
		{
			// Shader Program
			this->programID = glCreateProgram();
			GLuint vs = compileShader(vertexPath, shaderType::vertexShader);
			GLuint fs = compileShader(fragmentPath, shaderType::fragmentShader);
			GLuint gs;
			if (geometryPath != nullptr)
			{
				gs = compileShader(geometryPath, shaderType::geometryShader);
			}
			glAttachShader(this->programID, vs);
			glAttachShader(this->programID, fs);
			if (geometryPath != nullptr)
				glAttachShader(programID, gs);
			glLinkProgram(this->programID);

			// Print linking errors if any
			checkCompileErrors(programID, "PROGRAM");
			// delete the shaders as they're linked into our program now and no longer necessery
			deleteShader(vs);
			deleteShader(fs);
			if (geometryPath != nullptr)
				deleteShader(gs);
		}

		Shader(GLuint vertex, GLuint fragment, GLuint geometry = -1)
		{
			// Shader Program
			this->programID = glCreateProgram();
			glAttachShader(this->programID, vertex);
			glAttachShader(this->programID, fragment);
			if (geometry != -1)
			{
				glAttachShader(programID, geometry);
			}
			glLinkProgram(this->programID);

			// Print linking errors if any
			checkCompileErrors(programID, "PROGRAM");
		}

		Shader(const char* compShaderPath)
		{
			GLuint comp = compileShader(compShaderPath, shaderType::computeShader);

			// Shader Program
			this->programID = glCreateProgram();
			glAttachShader(programID, comp);
			glLinkProgram(this->programID);
			checkCompileErrors(programID, "PROGRAM");
			deleteShader(comp);
		}

		static GLuint compileShader(std::string Path, shaderType type) {
			// now use simpler method instead
			/*std::string Code;
			std::ifstream ShaderFile;
			// ensures ifstream objects can throw exceptions:
			ShaderFile.exceptions(std::ifstream::badbit);
			try
			{
				// Open files
				ShaderFile.open(Path);
				if (!ShaderFile.is_open()) {
					std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << Path << std::endl;
				}
				std::stringstream ShaderStream;
				// Read file's buffer contents into streams
				ShaderStream << ShaderFile.rdbuf();
				// close file handlers
				ShaderFile.close();
				// Convert stream into string
				Code = ShaderStream.str();
			}
			catch (std::ifstream::failure e)
			{
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << Path << std::endl;
			}
			const GLchar* ShaderCode = Code.c_str();
			*/
			std::string source = SourceFromFile(Path);
			const char* ShaderCode = source.c_str();
			GLuint shaderId;
			GLint success;
			GLchar infoLog[512];
			// Vertex Shader
			switch (type) {
			case vertexShader:
				shaderId = glCreateShader(GL_VERTEX_SHADER);
				break;
			case fragmentShader:
				shaderId = glCreateShader(GL_FRAGMENT_SHADER);
				break;
			case geometryShader:
				shaderId = glCreateShader(GL_GEOMETRY_SHADER);
				break;
			case computeShader:
				shaderId = glCreateShader(GL_COMPUTE_SHADER);
				break;
			}
			glShaderSource(shaderId, 1, &ShaderCode, NULL);
			glCompileShader(shaderId);
			// Print compile errors if any
			glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::COMPILATION_FAILED: " << Path << "\n" << infoLog << std::endl;
			}
			return shaderId;
		}

		static void deleteShader(GLuint shader) { glDeleteShader(shader); }

		GLuint getUniformLocation(const char *s) { return glGetUniformLocation(programID, s); }

		GLuint GetProgram()
		{
			return programID;
		}

		~Shader()
		{
			deleteShader(programID);
		}

		// activate the shader
		// ------------------------------------------------------------------------
		void use()
		{
			glUseProgram(programID);
		}
		// utility uniform functions
		// ------------------------------------------------------------------------
		void setBool(const std::string &name, bool value) const
		{
			glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
		}
		// ------------------------------------------------------------------------
		void setInt(const std::string &name, int value) const
		{
			glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
		}
		// ------------------------------------------------------------------------
		void setUInt(const std::string &name, unsigned int value) const
		{
			glUniform1ui(glGetUniformLocation(programID, name.c_str()), value);
		}
		// ------------------------------------------------------------------------
		void setFloat(const std::string &name, float value) const
		{
			glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
		}
		// ------------------------------------------------------------------------
		void setVec2(const std::string &name, const glm::vec2 &value) const
		{
			glUniform2fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
		}
		void setVec2(const std::string &name, float x, float y) const
		{
			glUniform2f(glGetUniformLocation(programID, name.c_str()), x, y);
		}
		// ------------------------------------------------------------------------
		void setVec3(const std::string &name, const glm::vec3 &value) const
		{
			glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
		}
		void setVec3(const std::string &name, float x, float y, float z) const
		{
			glUniform3f(glGetUniformLocation(programID, name.c_str()), x, y, z);
		}
		// ------------------------------------------------------------------------
		void setVec4(const std::string &name, const glm::vec4 &value) const
		{
			glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
		}
		void setVec4(const std::string &name, float x, float y, float z, float w)
		{
			glUniform4f(glGetUniformLocation(programID, name.c_str()), x, y, z, w);
		}
		// ------------------------------------------------------------------------
		void setMat2(const std::string &name, const glm::mat2 &mat) const
		{
			glUniformMatrix2fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}
		// ------------------------------------------------------------------------
		void setMat3(const std::string &name, const glm::mat3 &mat) const
		{
			glUniformMatrix3fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}
		// ------------------------------------------------------------------------
		void setMat4(const std::string &name, const glm::mat4 &mat) const
		{
			glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}

	private:
		static void checkCompileErrors(GLuint shader, std::string type)
		{
			GLint success;
			GLchar infoLog[1024];
			if (type != "PROGRAM")
			{
				glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
				if (!success)
				{
					glGetShaderInfoLog(shader, 1024, NULL, infoLog);
					std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				}
			}
			else
			{
				glGetProgramiv(shader, GL_LINK_STATUS, &success);
				if (!success)
				{
					glGetProgramInfoLog(shader, 1024, NULL, infoLog);
					std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				}
			}
		}

		
		GLuint programID;
	};
}