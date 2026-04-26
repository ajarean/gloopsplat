// Andy Jarean, Tingxuan Wu

/* ATTRIBUTIONS:
 * OpenGL boilerplate based on tutorials by Joey de Vries (https://learnopengl.com and https://github.com/JoeyDeVries/LearnOpenGL/)
 * License: CC BY-NC 4.0 (https://creativecommons.org/licenses/by-nc/4.0/)
 * Attributed to: Joey de Vries (Twitter: @JoeyDeVriez)
 */

 // specific tutorial(s) referenced: https://learnopengl.com/Getting-started/Shaders

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath);
    void use();
    // utility uniform functions
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string &name, const glm::mat4 &value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
};

#endif