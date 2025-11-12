#pragma once
#include "../nclgl/OGLRenderer.h"
#include <vector>
class HeightMap;
class Camera;
class Mesh;
class Light;
class MeshAnimation;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void DrawHeightMap();
	void DrawWater();
	void DrawSkybox();
	void DrawAnimations();
	void checkMeshes();
	void checkTextures();
	void checkShaders();
	void checkBuffers();
	void setCameraNodes();
	void setVariables();
	void toggleCamera() {if(isCameraFree != true){isCameraFree != isCameraFree;}};
	bool isCameraFree;
	HeightMap* heightMap;
	Shader* shader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* lightShader;
	Light* light;
	Camera* camera;
	Mesh* quad;
	Mesh* sphere;
	Mesh* Tree;
	MeshAnimation* animation;
	vector<GLuint> matTextures;
	GLuint terrainTex;
	GLuint waterTex;
	GLuint cubeMap;
	GLuint terrainBump;
	float waterRotate;
	float waterCycle;
};
