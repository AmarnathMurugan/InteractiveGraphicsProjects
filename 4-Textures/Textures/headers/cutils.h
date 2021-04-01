#ifndef CUTILS_H
#define CUTILS_H

//---Custom utilities---

#include<stdlib.h>
#include<fstream>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
float rand01()
{
	return (rand() % 10 + 1.0) / 10.0;
}

float lerp(float a, float b, float t)
{
	return t * b + (1 - t) * a;
}

std::string GetStringFromFile(const char* name)
{
	std::ifstream f(name);
	if (!f.is_open())			
		return "file not found";
	
	std::string content;
	std::string str="";

	while (!f.eof())
	{
		std::getline(f, str);
		content.append(str+"\n");
	}
	f.close();

	return content;
}

glm::vec3 UpAxis(0.0f, 1.0f, 0.0f), RightAxis(1.0f, 0.0f, 0.0f);




#endif // !CUTILS_H

