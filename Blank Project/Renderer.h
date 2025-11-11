#pragma once
#include "../nclgl/OGLRenderer.h"
#include <vector>
class HeightMap;
class Camera;
class Mesh;
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
	HeightMap* heightMap;
	Shader* shader;
	Camera* camera;
	Mesh* quad;
	Mesh* sphere;
	Mesh* Tree;
	MeshAnimation* animation;
	vector<GLuint> matTextures;
	GLuint terrainTex;
};
