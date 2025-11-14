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
	void toggleCamera() { isCameraFree = !isCameraFree; }

protected:
	void DrawHeightMap();
	void DrawWater(float transparancy);
	void DrawSkybox();
	void DrawAnimations();
	void DrawShadowScene(Light* l);
	void checkMeshes();
	void checkTextures();
	void checkShaders();
	void checkBuffers();
	void setCameraNodes();
	void setVariables();
	void checkAnimation();
	void checkModelMatrial();
	void checkCurrentCamera();
	void GenerateScreenTexture(GLuint &into, bool depth = false);
	void fillBuffers();
	void combineBuffers();
	void createPointLights();
	void Renderer::MoveLight(Vector3 position, Vector4 colour);
	bool isCameraFree;
	HeightMap* heightMap;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* lightShader;
	Shader* characterShader;
	Shader* shadowShader;
	Shader* sceneShader;
	Shader* pointLightShader;
	Shader* combineShader;
	Light* light;
	Light* pointLights;
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
	GLuint shadowTex;
	GLuint shadowFBO;
	GLuint bufferFBO;
	GLuint pointLightFBO;
	GLuint bufferColourTex;
	GLuint bufferNormalTex;
	GLuint bufferDepthTex;
	GLuint lightDiffuseTex;
	GLuint lightSpecularTex;
	Matrix4 soldierModel;
	Vector3 soldierPos;
	Matrix4 TreeModel;
	Vector3 TreePos;
	float waterRotate;
	float waterCycle;
	int currentFrame;
	float frameTime;
};
