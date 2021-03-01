#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<cy/cyTriMesh.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include "headers/cutils.h"
#include <map>

//OpenGL callbacks
void inputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseInputCallback(GLFWwindow* window, int key, int action, int mods);
void framebufferResizeCallback(GLFWwindow* window, int w, int h);

void processMesh();
void compileShaders(std::string vertShdrName, std::string fragShdrName,GLuint &program);
void setUniformLocations();
void initMVP();
void updateMVP();
void setShaderProperties();
void initModelBuffer();
void initOctahedronBuffer();
void mouseInputTransformations(GLFWwindow* window);

unsigned int width = 960, height = 540;

float yRot=0, xRot=0,yOffset=0,CamSpeed = 0.1f,camDist=0, lightScale = 0.2;
glm::mat4 persProjection, orthoProjection, model, view, mvp, mv, vectorRotationMatrix(1.0f), lightModelMatrix,lightMVP;
glm::mat3 mvNormal;
glm::dvec2 prevMousePos, curMousePos, deltaMousePos;
glm::vec2 CamDistLimit(0.5f, 5.0f);
glm::vec3 UpAxis(0.0f, 1.0f, 0.0f), RightAxis(1.0f, 0.0f, 0.0f), Center;

cy::TriMesh meshData;
std::vector<Vertdata> data;

//Material properties
glm::vec3 LightPos(0.0f, 1.5f, 0.0f), LightDir, ViewDir(0,3,-3), DiffuseColor(0.2f, 0.8f, 0.7f);
float LightIntensity = 1.0f, AmbientIntensity = 0.1f, Shininess = 50.0f;

bool isPerspective=true, isLeftMouseHeld = false, isRightMouseHeld = false, isCtrlHeld = false, isRecompile=false;

GLuint mvpLoc, mvLoc, lightDirLoc, viewDirLoc, lightMvpLoc;
GLuint diffuseColLoc, lightIntensityLoc, ambientIntensityLoc, shininessLoc;

GLuint lightVBO, lightVAO, lightEBO;
GLuint modelVBO, modelVAO, modelEBO;
GLuint ModelProgram, LightProgram;

int main(int argc, char* argv[])
{
		
	//Init GLFW and Versions
	if (!glfwInit())	
		return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	//Create window
	GLFWwindow* window = glfwCreateWindow(width, height, "Shading", NULL, NULL);
	if (window == NULL)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Unable to load GLAD";
		return -1;
	}

	//Set Callbacks
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	glfwSetMouseButtonCallback(window, mouseInputCallback);
	glfwSetKeyCallback(window, inputCallback);
	glEnable(GL_DEPTH_TEST);

	//Load model	
	if (!meshData.LoadFromFileObj(argv[1], false))
	{
		std::cout << "obj loading failed";
		return -1;
	}	
	std::cout << "obj loading complete \n";		
	
	processMesh();
	
	//Set program
	compileShaders("BlinnVert.glsl", "BlinnFrag.glsl",ModelProgram);
	if (ModelProgram == -1) return -1;
	compileShaders("UnlitVert.glsl", "UnlitFrag.glsl", LightProgram);

	//glUseProgram(ModelProgram);	



	setUniformLocations();

	//Set MVP matrix
	initMVP();

	setShaderProperties();
	
	//Set buffers
	
	initModelBuffer();

	initOctahedronBuffer();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	while (!glfwWindowShouldClose(window))
	{	
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (isRightMouseHeld || isLeftMouseHeld) mouseInputTransformations(window);
		
		glUseProgram(LightProgram);
		glBindVertexArray(lightVAO);
		glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);

		if (isRecompile)
		{
			glDeleteProgram(ModelProgram);
			compileShaders("BlinnVert.glsl", "BlinnFrag.glsl", ModelProgram);
			glUseProgram(ModelProgram);
			mvpLoc = glGetUniformLocation(ModelProgram, "MVP");
			glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);
			isRecompile = false;			
		}
		else
			glUseProgram(ModelProgram);
		
		glBindVertexArray(modelVAO);
		glDrawElements(GL_TRIANGLES, meshData.NF()*3, GL_UNSIGNED_INT, 0);

		

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &modelVAO);
	glDeleteBuffers(1, &modelVBO);
	glDeleteBuffers(1, &modelEBO);
	glDeleteProgram(ModelProgram);

	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &lightVBO);
	glDeleteBuffers(1, &lightEBO);
	glDeleteProgram(LightProgram);

	glfwTerminate();
	return 0;
}

void inputCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);	

	if(key==GLFW_KEY_P && action == GLFW_PRESS)
	{
		isPerspective = !isPerspective;
		updateMVP();
	}

	if (key == GLFW_KEY_LEFT_CONTROL)
	{
		if (action == GLFW_PRESS)
		{
			isCtrlHeld = true;
			glfwGetCursorPos(window, &prevMousePos.x, &prevMousePos.y);
		}
		else if (action == GLFW_RELEASE)
			isCtrlHeld = false;
	}

	if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
		isRecompile = true;

	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		std::cout << "\nLight Dir:" << LightDir.x << "," << LightDir.y << "," << LightDir.z;
		std::cout << "\nView Dir:" << ViewDir.x << "," << ViewDir.y << "," << ViewDir.z;
		std::cout<<"\nDot:"<<glm::dot(UpAxis, LightDir);
	}
}

void mouseInputCallback(GLFWwindow* window, int key, int action, int mods)
{
	if (key == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			isLeftMouseHeld = true;
			glfwGetCursorPos(window, &prevMousePos.x, &prevMousePos.y);
		}
		else if (action == GLFW_RELEASE)
			isLeftMouseHeld = false;
	}

	if (key == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &prevMousePos.x, &prevMousePos.y);
			isRightMouseHeld = true;
		}
		else if (action == GLFW_RELEASE)
			isRightMouseHeld = false;
	}
}

void framebufferResizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
	width = w;
	height = h;
	updateMVP();
}

void compileShaders(std::string vertShdrName, std::string fragShdrName, GLuint& program)
{
	GLuint vertShader, fragShader;
	//Read Vert Shader file and compile
	std::string path = "src/";
	std::string vertStr = GetStringFromFile((path+vertShdrName).c_str());
	const char* vert = vertStr.c_str();
	vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vert, NULL);
	glCompileShader(vertShader);
	int success;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		std::cout << "Vert compilation failed";
		return ;
	}

	//Read Frag Shader file and compile
	std::string fragStr = GetStringFromFile((path + fragShdrName).c_str());
	const char* frag = fragStr.c_str();
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &frag, NULL);
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		std::cout << "Frag compilation failed";
		return ;
	}
	
	//Create program and link 
	program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	char infoLog[512];
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cout << "Linking Failed:" << infoLog << std::endl;
	}
	std::cout << "Shader Compilation Complete \n";
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);	
}

void setUniformLocations()
{
	mvpLoc = glGetUniformLocation(ModelProgram, "MVP");
	mvLoc = glGetUniformLocation(ModelProgram, "MV");
	lightDirLoc = glGetUniformLocation(ModelProgram, "lightDir");
	viewDirLoc = glGetUniformLocation(ModelProgram, "viewDir");
	diffuseColLoc = glGetUniformLocation(ModelProgram, "diffuseCol");
	lightIntensityLoc = glGetUniformLocation(ModelProgram, "lightIntensity");
	ambientIntensityLoc = glGetUniformLocation(ModelProgram, "ambientIntensity");
	shininessLoc = glGetUniformLocation(ModelProgram, "shininess");
	lightMvpLoc = glGetUniformLocation(LightProgram, "MVP");
}

void initMVP()
{
	glUseProgram(ModelProgram);
	camDist = ViewDir.length();
	ViewDir = glm::normalize(ViewDir);
	persProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.5f, 10.0f);
	view = glm::lookAt(ViewDir*camDist, glm::vec3(0.0), UpAxis);
	//Scale model to make height 1, rotate to make it upright and center model to origin
	model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / (meshData.GetBoundMax().y - meshData.GetBoundMin().y)));
	model = glm::rotate(model, glm::radians(-90.0f), RightAxis);
	model = glm::translate(model,glm::vec3(0.0,0.0,-0.5f*(meshData.GetBoundMax().z-meshData.GetBoundMin().z)));
	mv =  view * model;
	mvp = persProjection * mv;

	//Set values in shader	
	
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);
	mvNormal = glm::transpose(glm::inverse(glm::mat3(mv)));
	glUniformMatrix3fv(mvLoc, 1, GL_FALSE, &mvNormal[0][0]);

	//Light mvp
	glUseProgram(LightProgram);
	lightModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(lightScale));
	lightModelMatrix = glm::translate(lightModelMatrix, LightPos/lightScale);
	lightMVP = persProjection * view * lightModelMatrix;
	glUniformMatrix4fv(lightMvpLoc, 1, GL_FALSE, &lightMVP[0][0]);
}

void updateMVP()
{
	//Updates perspective when window size changes
	persProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.5f, 10.0f);
	//Changes to cam pos through input
	view = glm::lookAt(ViewDir * camDist, glm::vec3(0.0), UpAxis);
	if (!isPerspective)
	{
		float multiplier = 5.0f;
		orthoProjection = glm::ortho(-multiplier * (float)width / (float)height, multiplier * (float)width / (float)height, -multiplier, multiplier, 0.1f, 10.0f);		
		mv = view * model;
		mvp = orthoProjection * view * model;
	}
	else
	{
		mv = view * model;
		mvp = persProjection * mv;
	}
	
	//Set values in shader	
	glUseProgram(ModelProgram);
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);
	mvNormal = glm::transpose(glm::inverse(glm::mat3(mv)));
	glUniformMatrix3fv(mvLoc, 1, GL_FALSE, &mvNormal[0][0]);

	//Light mvp
	glUseProgram(LightProgram);
	lightModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(lightScale));
	lightModelMatrix = glm::translate(lightModelMatrix, LightPos/lightScale);
	lightMVP = persProjection * view * lightModelMatrix;
	glUniformMatrix4fv(lightMvpLoc, 1, GL_FALSE, &lightMVP[0][0]);	
}

void setShaderProperties()
{
	glUseProgram(ModelProgram);
	glUniform3f(diffuseColLoc, DiffuseColor.x, DiffuseColor.y, DiffuseColor.z);
	glm::vec3 viewSpaceVec = glm::vec3(view * glm::vec4(ViewDir, 0.0f));
	glUniform3f(viewDirLoc, viewSpaceVec.x, viewSpaceVec.y, viewSpaceVec.z);
	LightDir = glm::normalize(LightPos);
	viewSpaceVec = glm::vec3(view * glm::vec4(LightDir, 0.0f));
	glUniform3f(lightDirLoc, viewSpaceVec.x, viewSpaceVec.y, viewSpaceVec.z);
	glUniform1f(lightIntensityLoc, LightIntensity);
	glUniform1f(ambientIntensityLoc, AmbientIntensity);
	glUniform1f(shininessLoc, Shininess);
}

void mouseInputTransformations(GLFWwindow* window)
{
	glfwGetCursorPos(window, &curMousePos.x, &curMousePos.y);
	deltaMousePos = curMousePos - prevMousePos;
	if (isRightMouseHeld)
	{ 
		if (isCtrlHeld)
		{
			vectorRotationMatrix = glm::rotate(glm::mat4(1.0), (float)deltaMousePos.y * 0.01f, glm::vec3(1.0f, 0.0f, 0.0f));		
			LightPos = glm::vec3((vectorRotationMatrix * glm::vec4(LightPos, 1.0f)));
			LightDir = glm::normalize(LightPos);
		}
		else
		{
			camDist += deltaMousePos.y * CamSpeed;
			camDist = camDist < CamDistLimit.x ? CamDistLimit.x : camDist;
			camDist = camDist > CamDistLimit.y ? CamDistLimit.y : camDist;
		}
	}
	if (isLeftMouseHeld)
	{
		vectorRotationMatrix = glm::rotate(glm::mat4(1.0), (float)deltaMousePos.x * -0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
		if (isCtrlHeld)
		{
			LightPos = glm::vec3((vectorRotationMatrix * glm::vec4(LightPos,1.0f)));
			LightDir = glm::normalize(LightPos);
		}
		else
		{			
			ViewDir = glm::vec3((vectorRotationMatrix * glm::vec4(ViewDir, 0.0f)));
			/*vectorRotationMatrix = glm::rotate(glm::mat4(1.0), (float)deltaMousePos.y * 0.01f, glm::vec3(1.0f, 0.0f, 0.0f));
			ViewDir = glm::vec3((vectorRotationMatrix * glm::vec4(ViewDir, 0.0f)));*/
		}
	}
	prevMousePos = curMousePos;
	updateMVP();
	setShaderProperties();
}

void processMesh()
{
	meshData.ComputeBoundingBox();
	if (meshData.IsBoundBoxReady())
	{
		cy::Vec3f center = (meshData.GetBoundMin() + meshData.GetBoundMax()) / 2.0f;
		Center = glm::vec3(center.x, center.y, center.z);
	}

	std::map<int, int> faceMap;
	std::map<std::pair<int, int>, int> modifiedVertMap;
	glm::vec3 tempPos, tempNorm;
	for (int i = 0; i < meshData.NV(); i++)
	{
		tempPos = glm::vec3(meshData.V(i).x, meshData.V(i).y, meshData.V(i).z);
		tempNorm = glm::vec3(meshData.VN(i).x, meshData.VN(i).y, meshData.VN(i).z);
		data.push_back(Vertdata{ tempPos, tempNorm });
	}

	//Duplicate vertices to avoid conflicts 
	int pos = 0;
	for (int i = 0; i < meshData.NF(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (faceMap.count(meshData.F(i).v[j]) == 0)
			{
				faceMap.emplace(meshData.F(i).v[j], meshData.FN(i).v[j]);
				pos = meshData.FN(i).v[j];
				data[meshData.F(i).v[j]].normal = glm::vec3(meshData.VN(pos).x, meshData.VN(pos).y, meshData.VN(pos).z);
			}
			else if (faceMap.at(meshData.F(i).v[j]) != meshData.FN(i).v[j])
			{
				if (modifiedVertMap.count(std::make_pair(meshData.F(i).v[j], meshData.FN(i).v[j])) > 0)
				{
					meshData.F(i).v[j] = modifiedVertMap.at(std::make_pair(meshData.F(i).v[j], meshData.FN(i).v[j]));
				}
				else
				{
					pos = meshData.F(i).v[j];
					tempPos = glm::vec3(meshData.V(pos).x, meshData.V(pos).y, meshData.V(pos).z);

					pos = meshData.FN(i).v[j];
					tempNorm = glm::vec3(meshData.VN(pos).x, meshData.VN(pos).y, meshData.VN(pos).z);

					data.push_back(Vertdata{ tempPos, tempNorm });
					meshData.F(i).v[j] = data.size() - 1;

					modifiedVertMap.emplace(std::make_pair(meshData.F(i).v[j], meshData.FN(i).v[j]), data.size() - 1);
					faceMap.emplace(data.size() - 1, meshData.FN(i).v[j]);
				}
			}
		}
	}
}

void initModelBuffer()
{
	glGenVertexArrays(1, &modelVAO);
	glBindVertexArray(modelVAO);

	glGenBuffers(1, &modelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertdata) * data.size(), &data[0], GL_STATIC_DRAW);

	glGenBuffers(1, &modelEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cy::TriMesh::TriFace) * meshData.NF(), &meshData.F(0), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)offsetof(Vertdata, normal));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void initOctahedronBuffer()
{
	float TetraPos[] =
	{
		0,0.75,0,
		-0.5,0,0.5,
		0.5,0,0.5,
		0.5,0,-0.5,
		-0.5,0,-0.5,
		0,-0.75,0
	};

	unsigned int TetraElems[] =
	{
		0,4,1,
		0,1,2,
		0,2,3,
		0,3,4,
		5,4,3,
		5,1,4,
		5,1,2,
		5,3,2
	};

	
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glGenBuffers(1, &lightVBO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TetraPos), TetraPos, GL_STATIC_DRAW);

	glGenBuffers(1, &lightEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TetraElems), TetraElems, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
}