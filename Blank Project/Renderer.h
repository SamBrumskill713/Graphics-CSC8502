#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Frustrum.h"
#include <vector>
class HeightMap;
class Camera;
class Mesh;
class Light;
class MeshAnimation;
class MeshMaterial;
class Matrix4;
class SceneNode;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;
	void toggleCamera() { 
		isCameraFree = !isCameraFree; 
	}
	void toggleScene() {
		isMainScene = !isMainScene;
		isTransScene = !isTransScene;
		if (isMainScene && !isTransScene) {
			activeCamera = mainSceneCamera;
		}
		else if (isTransScene && !isMainScene) {
			activeCamera = transSceneCamera;
		}
	}
	void togglePostProcessing() {
		isPostProcessing = !isPostProcessing;
	}

protected:
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
	void buildNodeLists(SceneNode* from);
	void sortNodeList();
	void drawNodes();
	void drawNode(SceneNode* n);
	void clearNodeLists();
	void checkAnimation();
	void checkModelMatrial();
	void checkCurrentCamera();
	void GenerateScreenTexture(GLuint &into, bool depth = false);
	void fillBuffers();
	void combineBuffers();
	void createPointLights();
	bool isCameraFree = true;
	bool isShadow = false;
	bool isMainScene = true;
	bool isTransScene = false;
	bool isPostProcessing = false;
	HeightMap* heightMap;
	HeightMap* heightMap2;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* lightShader;
	Shader* characterShader;
	Shader* shadowShader;
	Shader* sceneShader;
	Shader* pointLightShader;
	Shader* combineShader;
	Shader* nodeShader;
	Shader* characterShadowShader;
	Shader* processShader;
	Light* light;
	Light* pointLights;
	Camera* activeCamera;
	Camera* mainSceneCamera;
	Camera* transSceneCamera;
	Mesh* quad;
	Mesh* sphere;
	Mesh* Tree;
	Mesh* soldier;
	Mesh* UFO;
	Mesh* awesomeSkeleton;
	Mesh* houseMesh;
	Mesh* cube;
	SceneNode* landMapRoot;
	SceneNode* transSceneRoot;
	SceneNode* cubeNode = nullptr;
	MeshAnimation* soldierAnimation;	
	MeshAnimation* awesomeSkeletonAnimation;
	MeshMaterial* soldierMaterial;
	MeshMaterial* TreeMaterial;
	MeshMaterial* UFOMaterial;
	MeshMaterial* awesomeSkeletonMaterial;
	MeshMaterial* houseMaterial;
	vector<GLuint> soldierMatTextures;
	vector<GLuint> TreeMatTextures;
	vector<GLuint> UFOMatTextures;
	vector<GLuint> awesomeSkeletonMatTextures;
	vector<GLuint> houseMatTextures;
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
	GLuint cubeTexture;
	Matrix4 soldierModel;
	Vector3 soldierPos;
	Matrix4 TreeModel;
	Vector3 TreePos;
	Matrix4 UFOModel;
	Vector3 UFOPos;
	Matrix4 skelModel;
	Vector3 skelPos;
	Vector3 cubePos;
	Matrix4 cubeModel;
	float cubeAngle = 0.0f;
	float cubeRotateSpeed = 20.0f;
	float waterRotate;
	float waterCycle;
	int currentFrame;
	float frameTime;
	int direction;	
	Frustrum frameFrustrum;
	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
};
