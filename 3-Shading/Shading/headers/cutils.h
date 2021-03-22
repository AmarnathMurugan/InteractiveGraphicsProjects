#ifndef CUTILS_H
#define CUTILS_H

#include<stdlib.h>
#include<fstream>
class Color
{
	public:
		float values[3];
		Color():values{ 0,0,0 } {}
		inline float r() const { return values[0]; }
		inline float g() const { return values[1]; }
		inline float b() const { return values[2]; }
};

float rand01()
{
	
	return (rand() % 10 + 1.0) / 10.0;
}

float lerp(float a, float b, float t)
{
	return t * b + (1 - t) * a;
}

Color GetRandomColor()
{
	Color c;
	for (int i = 0; i < 3; i++)
		c.values[i] = rand01();
	return c;
}

void lerpColor(const Color& a, const Color& b,const float& t, Color& target)
{
	for (int i = 0; i < 3; i++)
		target.values[i] = lerp(a.values[i],b.values[i],t);
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

struct Vertdata
{
	glm::vec3 position, normal;
};

#endif // !CUTILS_H

