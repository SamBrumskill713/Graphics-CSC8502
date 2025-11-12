#pragma once
#include "../nclgl/OGLRenderer.h"
#include <array>
#include <vector>

class Camera;
class Mesh;

class Renderer : public OGLRenderer {
	public:
	Renderer(Window& parent);
	~Renderer(void);
	void UpdateScene(float dt) override;
	void RenderScene() override;
protected:
	void DrawShadowScene();
	void DrawMainScene();
	void MoveLight(Vector3 position, Vector4 colour);

	GLuint shadowTex;
	GLuint shadowTex2;
	GLuint shadowFBO;

	GLuint sceneDiffuse;
	GLuint sceneBump;
	float sceneTime;

	Shader* sceneShader;
	Shader* shadowShader;

	vector<Mesh*> sceneMeshes;
	vector<Matrix4> sceneTransforms;

	Camera* camera;
	Light* light;
	Light* light2;	
	//std::vector<Light*> lights;
};