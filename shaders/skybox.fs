/* ATTRIBUTIONS:
 * OpenGL boilerplate based on tutorials by Joey de Vries (https://learnopengl.com and https://github.com/JoeyDeVries/LearnOpenGL/)
 * License: CC BY-NC 4.0 (https://creativecommons.org/licenses/by-nc/4.0/)
 * Attributed to: Joey de Vries (Twitter: @JoeyDeVriez)
 */

// https://learnopengl.com/code_viewer_gh.php?code=src/4.advanced_opengl/6.2.cubemaps_environment_mapping/6.2.skybox.fs

#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, TexCoords);
}