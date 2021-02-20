#version 330 core

in vec3 col;
out vec4 FragCol;

void main()
{
	FragCol = vec4(col, 1.0f);
}