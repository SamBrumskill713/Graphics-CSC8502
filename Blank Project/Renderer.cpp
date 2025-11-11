#include "Renderer.h"
#include "../nclgl/camera.h"
#include "../nclgl/HeightMap.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	checkTextures();
	checkShaders();
	heightMap = new HeightMap(TEXTUREDIR"swampHeightmap.png");
	camera = new Camera(-40, 180, Vector3());

	Vector3 dimensions = heightMap->GetHeightmapSize();
	camera->SetPosition(dimensions * Vector3(0.5, 10, 0.5 / 2.0));

	SetTextureRepeating(terrainTex, true);

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	init = true;
}

Renderer::~Renderer(void) {
	delete heightMap;
	delete camera;
	delete shader;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	BindShader(shader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainTex);
	heightMap->Draw();
}

void Renderer::DrawAnimations()
{
}

void Renderer::DrawWater()
{
}

void Renderer::DrawSkybox() 
{
}

void Renderer::DrawHeightMap() 
{
	
}

void Renderer::checkMeshes()
{
}

void Renderer::checkTextures()
{
	terrainTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	if (!terrainTex) {
		return;
	}
}

void Renderer::checkShaders()
{
	shader = new Shader("TexturedVertex.glsl", "texturedfragment.glsl");

	if (!shader->LoadSuccess()) {
		return;
	}
}

void Renderer::checkBuffers()
{
}

void Renderer::setCameraNodes()
{
}

void Renderer::setVariables()
{
}