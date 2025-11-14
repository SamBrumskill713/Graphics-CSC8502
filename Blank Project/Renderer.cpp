#include "Renderer.h"
#include "../nclgl/camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Light.h"
#include "../nclgl/Mesh.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include "../nclgl/Matrix4.h"
#include <iostream>
#include <vector>

#define SHADOWSIZE 2048
const int LIGHT_NUM = 2;

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	checkMeshes();
	checkModelMatrial();
	checkAnimation();
	checkTextures();
	checkShaders();
	setVariables();
	GenerateScreenTexture(bufferDepthTex, true);
	GenerateScreenTexture(bufferColourTex);
	GenerateScreenTexture(bufferNormalTex);
	GenerateScreenTexture(lightDiffuseTex);
	GenerateScreenTexture(lightSpecularTex);
	checkBuffers();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	init = true;
}

Renderer::~Renderer(void) {
	delete heightMap;
	delete activeCamera;
	//delete freeCamera;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete characterShader;
	delete light;
	delete soldier;
	delete soldierAnimation;
	delete soldierMaterial;
	delete shadowShader;
	glDeleteTextures(1, &bufferColourTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &lightDiffuseTex);
	glDeleteTextures(1, &lightSpecularTex);
	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &pointLightFBO);
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
}

void Renderer::UpdateScene(float dt) {
	activeCamera->UpdateCamera(dt);
	viewMatrix = activeCamera->BuildViewMatrix();
	waterRotate += dt;
	waterCycle += dt;
	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % soldierAnimation->GetFrameCount();
		frameTime += 1.0f / soldierAnimation->GetFrameRate();
	}
	MoveLight(light->GetPosition(), light->GetColour());
	checkCurrentCamera();
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//DrawShadowScene(light);
	//fillBuffers();
	//createPointLights();
	DrawSkybox();
	//combineBuffers();
	viewMatrix = activeCamera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f,
		(float)width / (float)height,
		45.0f);
	DrawSkybox();
	DrawHeightMap();
	DrawWater(0.1f);
	DrawAnimations();
}

void Renderer::DrawWater(float transparancy)
{
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1,
		(float*)&activeCamera->GetPosition());
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);
	glUniform1f(glGetUniformLocation(reflectShader->GetProgram(), "transparancy"), 
		transparancy);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightmapSize();

	modelMatrix =
		Matrix4::Translation(hSize * 0.5f) *
		Matrix4::Scale(hSize * 0.5f) *
		Matrix4::Rotation(90, Vector3(1, 0, 0));

	textureMatrix =
		Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
		Matrix4::Scale(Vector3(10, 10, 10)) *
		Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	SetShaderLight(*light);
	quad->Draw();
}

void Renderer::DrawSkybox() 
{
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightMap() 
{
	BindShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1,
		(float*)&activeCamera->GetPosition());

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, terrainBump);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	heightMap->Draw();
}

void Renderer::checkTextures()
{
	terrainTex = SOIL_load_OGL_texture(TEXTUREDIR"Swampland.jpg", 
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	
	terrainBump = SOIL_load_OGL_texture(TEXTUREDIR"SwamplandDOT3.jpg", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"right.png", TEXTUREDIR"left.png",
		TEXTUREDIR"top.png", TEXTUREDIR"bottom.png",
		TEXTUREDIR"front.png", TEXTUREDIR"back.png",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	for (int i = 0; i < soldier->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = soldierMaterial->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(),
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		soldierMatTextures.emplace_back(texID);
	}

	if (!terrainTex || !waterTex || !terrainBump || !cubeMap) {
		return;
	}

	SetTextureRepeating(terrainTex, true);
	SetTextureRepeating(terrainBump, true);
	SetTextureRepeating(waterTex, true);
}

void Renderer::checkShaders()
{
	//shader = new Shader("TexturedVertex.glsl", "texturedfragment.glsl");
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");
	characterShader = new Shader("SkinningVertex.glsl", "texturedfragment.glsl");
	shadowShader = new Shader("shadowVertex.glsl", "shadowFragment.glsl");
	sceneShader = new Shader("BumpVertex.glsl", "bufferFragment.glsl");
	pointLightShader = new Shader("pointlightvert.glsl", "pointLightFrag.glsl");
	combineShader = new Shader("combineVert.glsl", "combineFrag.glsl");

	if (!reflectShader->LoadSuccess() ||
		!skyboxShader->LoadSuccess() ||
		!lightShader->LoadSuccess() ||
		!characterShader->LoadSuccess() ||
		!shadowShader->LoadSuccess() ||
		!sceneShader->LoadSuccess() ||
		!pointLightShader->LoadSuccess() ||
		!combineShader->LoadSuccess()) {
		return;
	}
}

void Renderer::checkBuffers()
{
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE,
		SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		shadowTex, 0);
	glDrawBuffer(GL_NONE);

	GLenum buffers[2] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &pointLightFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		bufferColourTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
		bufferNormalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		bufferDepthTex, 0);
	glDrawBuffers(2, buffers);

	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		lightDiffuseTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
		lightSpecularTex, 0);
	glDrawBuffers(2, buffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::setCameraNodes()
{
}

void Renderer::setVariables()
{
	activeCamera = new Camera(-40, 180, Vector3());
	Vector3 dimensions = heightMap->GetHeightmapSize();
	activeCamera->SetPosition(dimensions * Vector3(0.5, 10, 0.5 / 2.0));
	isCameraFree = false;
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, 
		(float)width / (float)height, 45.0f);
	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f),
		Vector4(1, 1, 1, 1), heightmapSize.x);
	pointLights = new Light[LIGHT_NUM];
	soldierPos = Vector3(heightmapSize.x / 2, heightmapSize.y, heightmapSize.z/2);
	soldierModel = Matrix4::Translation(soldierPos) * 
		Matrix4::Scale(Vector3(100, 100, 100));
	for (int i = 0; i < LIGHT_NUM; ++i) {
		Light& l = pointLights[i];
		l.SetPosition(Vector3(rand() % (int)heightmapSize.x, 150.0f,
			rand() % (int)heightmapSize.z));

		l.SetColour(Vector4(0.5f + (float)(rand() / (float)RAND_MAX),
			0.5f + (float)(rand() / (float)RAND_MAX),
			0.5f + (float)(rand() / (float)RAND_MAX),
			1));

		l.SetRadius(250.0f + (rand() % 250));
	}
	waterRotate = 0.0f;
	waterCycle = 0.0f;
	currentFrame = 0;
	frameTime = 0.0f;
}

void Renderer::checkMeshes() {
	quad = Mesh::GenerateQuad();
	heightMap = new HeightMap(TEXTUREDIR"swampHeightmap.png");
	soldier = Mesh::LoadFromMeshFile("Role_T.msh");
	sphere = Mesh::LoadFromMeshFile("Sphere.msh");
	if (heightMap == nullptr) {
		std::cout << "No model found";
		return;
	}
}

void Renderer::checkModelMatrial() {
	soldierMaterial = new MeshMaterial("Role_T.mat");
	if (soldierMaterial == nullptr) {
		std::cout << "No material found \n";
		return;
	}
}

void Renderer::checkAnimation() {
	soldierAnimation = new MeshAnimation("Role_T.anm");
	if (soldierAnimation == nullptr) {
		std::cout << "No animation found \n";
		return;
	}
}

void Renderer::DrawAnimations() {
	BindShader(characterShader);
	glUniform1i(glGetUniformLocation(characterShader->GetProgram(), "diffuseTex"), 0);

	modelMatrix = soldierModel;

	UpdateShaderMatrices();

	vector<Matrix4> frameMatrices;

	const Matrix4* invBindPose = soldier->GetInverseBindPose();
	const Matrix4* frameData = soldierAnimation->GetJointData(currentFrame);

	for (unsigned int i = 0; i < soldier->GetJointCount(); ++i) {
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(characterShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	for (int i = 0; i < soldier->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, soldierMatTextures[i]);
		soldier->DrawSubMesh(i);
	}
}

void Renderer::DrawShadowScene(Light* l)
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glCullFace(GL_FRONT);

	BindShader(shadowShader);

	viewMatrix = Matrix4::BuildViewMatrix(l->GetPosition(), Vector3(0, 0, 0));

	projMatrix = Matrix4::Perspective(1, 100, 1, 45);
	shadowMatrix = projMatrix * viewMatrix;

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();
	UpdateShaderMatrices();
	heightMap->Draw();

	//DrawAnimations();

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK);
}

void Renderer::checkCurrentCamera()
{
	if (isCameraFree) {
		//activeCamera = freeCamera;
	}
}

void Renderer::GenerateScreenTexture(GLuint& into, bool depth)
{
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint format = depth ? GL_DEPTH_COMPONENT24 : GL_RGBA8;
	GLuint type = depth ? GL_DEPTH_COMPONENT : GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, type,
		GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::fillBuffers()
{
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(sceneShader);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "bumpTex"), 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainTex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, terrainBump);

	modelMatrix.ToIdentity();
	viewMatrix = activeCamera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	UpdateShaderMatrices();

	heightMap->Draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::combineBuffers()
{
	BindShader(combineShader);
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "diffuseLight"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lightDiffuseTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "specularLights"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);

	quad->Draw();
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::createPointLights()
{
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	BindShader(pointLightShader);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);

	glUniform1i(glGetUniformLocation(pointLightShader->GetProgram(), "depthTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glUniform1i(glGetUniformLocation(pointLightShader->GetProgram(), "normTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform3fv(glGetUniformLocation(pointLightShader->GetProgram(), "cameraPos"),
		1, (float*)&activeCamera->GetPosition());

	glUniform2f(glGetUniformLocation(pointLightShader->GetProgram(), "pixelSize"),
		1.0f / width, 1.0f / height);

	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(pointLightShader->GetProgram(),
		"inverseProjView"), 1, false, invViewProj.values);
	UpdateShaderMatrices();
	for (int i = 0; i < LIGHT_NUM; ++i) {
		Light& l = pointLights[i];
		SetShaderLight(l);
		sphere->Draw();
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	glDepthMask(GL_TRUE);

	glClearColor(0.2f, 0.2f, 0.2f, 1);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::MoveLight(Vector3 position, Vector4 colour)
{
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_J)) {
		position.x -= 1.0f;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_L)) {
		position.x += 1.0f;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_I)) {
		position.z -= 1.0f;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_K)) {
		position.z += 1.0f;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT)) {
		colour.x -= 1.0f;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT)) {
		colour.x += 1.0f;
	}
	light->SetPosition(position);
	light->SetColour(colour);
}