#pragma once
#include "../nclgl/OGLRenderer.h"
#include <vector>
class HeightMap;
class Camera;
class Mesh;
class Light;
class MeshAnimation;
class MeshMaterial;
class Matrix4;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;
	void toggleCamera() { isCameraFree != isCameraFree; }

protected:
	void DrawHeightMap();
	void DrawWater(float transparancy);
	void DrawSkybox();
	void DrawAnimations();
	void checkMeshes();
	void checkTextures();
	void checkShaders();
	void checkBuffers();
	void setCameraNodes();
	void setVariables();
	void checkAnimation();
	void checkModelMatrial();
	void checkCurrentCamera();
	void Renderer::MoveLight(Vector3 position, Vector4 colour);
	bool isCameraFree;
	HeightMap* heightMap;
	Shader* shader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* lightShader;
	Shader* characterShader;
	Light* light;
	Camera* activeCamera;
	Camera* freeCamera;
	Mesh* quad;
	Mesh* sphere;
	Mesh* Tree;
	Mesh* soldier;
	MeshAnimation* soldierAnimation;	
	MeshMaterial* soldierMaterial;
	vector<GLuint> soldierMatTextures;
	GLuint terrainTex;
	GLuint waterTex;
	GLuint cubeMap;
	GLuint terrainBump;
	Matrix4 soldierModel;
	Vector3 soldierPos;
	Matrix4 TreeModel;
	Vector3 TreePos;
	float waterRotate;
	float waterCycle;
	int currentFrame;
	float frameTime;
};
