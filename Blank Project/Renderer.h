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
class SceneNode;
class Frustrum;

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
	void setNodes();
	void buildNodeList();
	void sortNodeList();
	void drawNodes();
	void drawNode();
	void checkAnimation();
	void checkModelMatrial();
	void checkCurrentCamera();
	void GenerateScreenTexture(GLuint &into, bool depth = false);
	void fillBuffers();
	void combineBuffers();
	void createPointLights();
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
	Shader* gbufferCharacterShader;
	Light* light;
	Light* pointLights;
	Camera* activeCamera;
	Camera* freeCamera;
	Mesh* quad;
	Mesh* sphere;
	Mesh* Tree;
	Mesh* soldier;
	Mesh* UFO;
	SceneNode* landMapRoot;
	SceneNode* UFOLightRoot;
	MeshAnimation* soldierAnimation;	
	MeshMaterial* soldierMaterial;
	MeshMaterial* TreeMaterial;
	MeshMaterial* UFOMaterial;
	vector<GLuint> soldierMatTextures;
	vector<GLuint> treeMatTextures;
	vector<GLuint> UFOMatTextures;
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
	Matrix4 UFOModel;
	Vector3 UFOPos;
	float waterRotate;
	float waterCycle;
	int currentFrame;
	float frameTime;
};
