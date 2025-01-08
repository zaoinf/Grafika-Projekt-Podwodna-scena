#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"

#include "Box.cpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <initializer_list>

namespace textures 
{

	GLuint terrain;
	GLuint shipNormal;
}


GLuint program;
GLuint programSun;
GLuint programTex;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;

Core::RenderContext cube; 
Core::RenderContext terrain;


glm::vec3 cameraPos = glm::vec3(-4.f, 0, 0);
glm::vec3 cameraDir = glm::vec3(1.f, 0.f, 0.f);


glm::vec3 spaceshipPos = glm::vec3(0.0f, 1.000000f, 0.0f);
glm::vec3 spaceshipDir = glm::vec3(-0.354510f, 0.000000f, 0.935054f);

GLuint VAO,VBO;

float aspectRatio = 1.f;

float exposition = 1.f;

glm::vec3 lightPos = glm::vec3(0, 10, 0);
glm::vec3 lightColor = glm::vec3(0.9, 0.7, 0.8)*100;

glm::vec3 spotlightPos = glm::vec3(0, 0, 0);
glm::vec3 spotlightConeDir = glm::vec3(0, 0, 0);
glm::vec3 spotlightColor = glm::vec3(0.5, 0.9, 0.8)*10;
float spotlightPhi = 3.14 / 3;



struct ModelWithTexture
{
	std::vector<GLuint> TexturesIDs;
	GLuint program;
	Core::RenderContext Context;
	glm::vec3 PositionOfTheModel;
	unsigned short TextureVertexSize = 1;
};

std::vector<ModelWithTexture>VectorOfModels;


glm::mat4 createCameraMatrix()
{
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir,glm::vec3(0.f,1.f,0.f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide,cameraDir));
	glm::mat4 cameraRotrationMatrix = glm::mat4({
		cameraSide.x,cameraSide.y,cameraSide.z,0,
		cameraUp.x,cameraUp.y,cameraUp.z ,0,
		-cameraDir.x,-cameraDir.y,-cameraDir.z,0,
		0.,0.,0.,1.,
		});
	cameraRotrationMatrix = glm::transpose(cameraRotrationMatrix);
	glm::mat4 cameraMatrix = cameraRotrationMatrix * glm::translate(-cameraPos);

	return cameraMatrix;
}

glm::mat4 createPerspectiveMatrix()
{
	
	glm::mat4 perspectiveMatrix;
	float n = 0.05;
	float f = 20.;
	float a1 = glm::min(aspectRatio, 1.f);
	float a2 = glm::min(1 / aspectRatio, 1.f);
	perspectiveMatrix = glm::mat4({
		1,0.,0.,0.,
		0.,aspectRatio,0.,0.,
		0.,0.,(f+n) / (n - f),2*f * n / (n - f),
		0.,0.,-1.,0.,
		});

	
	perspectiveMatrix=glm::transpose(perspectiveMatrix);

	return perspectiveMatrix;
}
float lastTime = -1.f;
float deltaTime = 0.f;

void updateDeltaTime(float time) {
	if (lastTime < 0) {
		lastTime = time;
		return;
	}

	deltaTime = time - lastTime;
	if (deltaTime > 0.1) deltaTime = 0.1;
	lastTime = time;
}
void DrawObjectWithTextureFromStruct(ModelWithTexture model) 
{
	GLuint ShaderInUseForTextures = model.program;
	glm::mat4 modelMatrix = glm::translate(model.PositionOfTheModel);
	glUseProgram(ShaderInUseForTextures);
	
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(ShaderInUseForTextures, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(ShaderInUseForTextures, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(ShaderInUseForTextures, "lightPos"), 0, 100, 0);

	Core::SetActiveTexture(model.TexturesIDs[0], "baseTexture", ShaderInUseForTextures, 0);
	if (model.TextureVertexSize==2)
	{
		Core::SetActiveTexture(model.TexturesIDs[1], "normalTexture", ShaderInUseForTextures, 1);
	}
	else if (model.TextureVertexSize == 3)
	{
		Core::SetActiveTexture(model.TexturesIDs[2], "metalicTexture", ShaderInUseForTextures, 2);
	}
	else if (model.TextureVertexSize == 4)
	{
		Core::SetActiveTexture(model.TexturesIDs[3], "otherTexture", ShaderInUseForTextures, 3);
	}
	Core::DrawContext(model.Context);

}
void drawObjectTexture(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID) {
	glUseProgram(programTex);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programTex, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programTex, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(programTex, "lightPos"), 0, 0, 0);

	Core::SetActiveTexture(textureID, "baseTexture", programTex, 0);
	Core::DrawContext(context);

}

void drawObjectColor(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color) {
	
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform1f(glGetUniformLocation(program, "exposition"), exposition);

	glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);

	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glUniform3f(glGetUniformLocation(program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glGetUniformLocation(program, "lightColor"), lightColor.x, lightColor.y, lightColor.z);

	/*glUniform3f(glGetUniformLocation(program, "spotlightConeDir"), spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	glUniform3f(glGetUniformLocation(program, "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
	glUniform3f(glGetUniformLocation(program, "spotlightColor"), spotlightColor.x, spotlightColor.y, spotlightColor.z);
	glUniform1f(glGetUniformLocation(program, "spotlightPhi"), spotlightPhi);
	*/


	Core::DrawContext(context);

}
void RenderAllModdelsWithTextures()
{
	for (unsigned int i = 0; i < VectorOfModels.size(); ++i)
	{
		DrawObjectWithTextureFromStruct(VectorOfModels[i]);
	}
}
void renderScene(GLFWwindow* window)
{
	glClearColor(0.4f, 0.4f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 transformation;
	float time = glfwGetTime();
	updateDeltaTime(time);



	glUseProgram(program);
	
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,0.,0.,0.,1.,});


	drawObjectColor(shipContext,glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.1)),glm::vec3(1.0f, 0.0f, 0.0f));

	RenderAllModdelsWithTextures();//Renderuje wszystkie modele


	//drawObjectTexture(cube, glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)),textures::cube);
	//drawObjectTexture(terrain, glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)), textures::terrain);



	spotlightPos = spaceshipPos + 0.5 * spaceshipDir;
	spotlightConeDir = spaceshipDir;

	glUseProgram(0);
	glfwSwapBuffers(window);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
}
void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);// przekazuje scene do inicjalizacji kontekstu
}
void AddModelWithTextureToVectorOfModels(glm::vec3 PositionOfTheModel, std::string PathToTheModel, std::initializer_list <const char*> PathsToTheTextures, const char* ShaderVariableName, GLuint program)
{
	ModelWithTexture model;

	for (auto Path : PathsToTheTextures)
	{
		model.TexturesIDs.push_back(Core::LoadTexture(Path));
	}
	model.TextureVertexSize = model.TexturesIDs.size();
	loadModelToContext(PathToTheModel, model.Context);
	model.program = program;
	model.PositionOfTheModel = PositionOfTheModel;

	VectorOfModels.push_back(model);
}


void init(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/main_shader.vert", "shaders/main_shader.frag");
	programSun = shaderLoader.CreateProgram("shaders/sun.vert", "shaders/sun.frag");
	programTex = shaderLoader.CreateProgram("shaders/texture.vert", "shaders/texture.frag");


	loadModelToContext("./models/spaceship.obj", shipContext);

	AddModelWithTextureToVectorOfModels(glm::vec3(0.0f, 0.0f, 0.0f), "./models/cube.obj", { "./img/Red_cube.png" }, "cube_Texture", programTex);
	AddModelWithTextureToVectorOfModels(glm::vec3(0.0f, 0.0f, 0.0f), "./models/SeaFloor.obj", { "./img/SandSeaFloor2.png" }, "floor", programTex);

	
	
	//AddModelWithTextureToVectorOfModels(glm::vec3(1.0f, 1.0f, 1.0f), "./models/sand_cube.obj", { "./img/sand3.jpeg" }, "cube_Texture2", programTex);
	//AddModelWithTextureToVectorOfModels(glm::vec3(3.0f, 3.0f, 3.0f), "./models/SkeletonBody.obj", {"./img/SkeletonTextures/SkeletonBody_Base_Color.png","./img/SkeletonTextures/SkeletonBody_Normal_OpenGL.png" }, "SkeletonBody", programTex);
	//AddModelWithTextureToVectorOfModels(glm::vec3(0.0f, -1.0f, 0.0f), "./models/experimental_floor.obj",{ "./img/textures/Ground_baseColor.jpeg","./img/textures/Ground_normal.jpeg" ,"./img/textures/Ground_metallicRoughness.jpeg" }, "floor", programTex);
	//AddModelWithTextureToVectorOfModels(glm::vec3(3.0f, 3.0f, 3.0f), "./models/Skull.obj", { "./img/SkullTextures/Skull_Base_Color.png","./img/SkeletonTextures/Skull_Normal_OpenGL.png" }, "SkeletonBody", programTex);

	
	
		
	//textures::cube = Core::LoadTexture("./img/Red_cube.png");
	//Core::SetActiveTexture(textures::cube, "cube_Texture", programTex, 0);


	//loadModelToContext("./models/ocean_floor_scene.glb", terrain);
	//textures::terrain = Core::LoadTexture("./img/textures/maps/terrain with river.fbx_lambert1_BaseColor.png");
	//Core::SetActiveTexture(textures::terrain, "trrain_texture", programTex, 0);
}

void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(programTex);
	shaderLoader.DeleteProgram(program);
}

//obsluga wejscia przyciski na klawiaturze it takie tam
void processInput(GLFWwindow* window)
{
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::vec3(0.f, 1.f, 0.f);
	float angleSpeed = 0.05f * deltaTime * 60;
	float moveSpeed = 0.05f * deltaTime * 60;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		spaceshipPos += spaceshipDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		spaceshipPos -= spaceshipDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		spaceshipPos += spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		spaceshipPos -= spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		spaceshipPos += spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		spaceshipPos -= spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		spaceshipDir = glm::vec3(glm::eulerAngleY(angleSpeed) * glm::vec4(spaceshipDir, 0));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		spaceshipDir = glm::vec3(glm::eulerAngleY(-angleSpeed) * glm::vec4(spaceshipDir, 0));

	cameraPos = spaceshipPos - 1.5 * spaceshipDir + glm::vec3(0, 1, 0) * 0.5f;
	cameraDir = spaceshipDir;

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		exposition -= 0.05;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		exposition += 0.05;

	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		printf("spaceshipPos = glm::vec3(%ff, %ff, %ff);\n", spaceshipPos.x, spaceshipPos.y, spaceshipPos.z);
		printf("spaceshipDir = glm::vec3(%ff, %ff, %ff);\n", spaceshipDir.x, spaceshipDir.y, spaceshipDir.z);
	}

}
// funkcja jest glowna petla
void renderLoop(GLFWwindow* window) 
{
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		renderScene(window);
		glfwPollEvents();
	}
}
