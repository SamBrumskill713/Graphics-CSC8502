#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Mesh.h"
#include "HeightMap.h"
#include "MeshAnimation.h"
#include <vector>

class SceneNode {
public:
	SceneNode();
	SceneNode(Mesh* m, Vector4 colour = Vector4(1, 1, 1, 1));
	SceneNode(HeightMap* heightmap, Vector4 colour = Vector4(1, 1, 1, 1));
	~SceneNode(void);

	void setShaderTextures();

	void SetTransform(const Matrix4& matrix) { transform = matrix; }
	const Matrix4& GetTransform() const { return transform; }
	Matrix4 GetWorldTransform() const { return worldTransform; }

	Vector4 GetColour() const { return colour; }
	void SetColour(Vector4 c) { colour = c; }

	Vector3 GetModelScale() const { return modelScale; }
	void SetModelScale(Vector3 s) { modelScale = s; }

	Mesh* GetMesh() const { return mesh; }
	void setMesh(Mesh* m) { mesh = m; }

	void AddChild(SceneNode* s);
	void RemoveChild(int index);

	virtual void Update(float dt);
	
	virtual void Draw(const OGLRenderer& r);

	std::vector<SceneNode*>::const_iterator GetChildIteratorStart() 
	{ return children.begin(); }
	std::vector<SceneNode*>::const_iterator GetChildIteratorEnd() 
	{ return children.end(); }

	float GetBoundingRadius() const { return boundingRadius; }
	void SetBoundingRadius(float f) { boundingRadius = f; }

	float GetCameraDistance() const { return distanceFromCamera; }
	void SetCameraDistance(float f) { distanceFromCamera = f; }

	void AddTexture(GLuint tex) { textures.emplace_back(tex); }
	GLuint GetTexture(int i) const { return textures[i]; }
	int getTextureSize() const { return textures.size(); }

	void SetBumpMap(GLuint bump) { bumpMap = bump; }
	GLuint GetBumpMap() const { bumpMap; }

	void AddMatTexture(GLuint tex) { matTextures.emplace_back(tex); }
	void SetMatTextures(std::vector<GLuint> tex) { matTextures = tex; }
	GLuint GetMatTexture(int i) const { return matTextures[i]; }
	int getMatTextureSize() const { return matTextures.size(); }

	void SetShader(Shader* s) { shader = s; }
	Shader* GetShader() const { return shader; }

	HeightMap* GetHeightMap() const { return heightMap; }

	void SetAnimation(MeshAnimation* anim) { animation = anim; }
	MeshAnimation* GetAnimation() const { return animation; }

	static bool CompareByCameraDistance(SceneNode* a, SceneNode* b) {
		return (a->distanceFromCamera < b->distanceFromCamera) ? true : false;
	}


protected:
	SceneNode* parent;
	Mesh* mesh;
	Shader* shader;
	HeightMap* heightMap;
	MeshAnimation* animation;
	Matrix4 worldTransform;
	Matrix4 transform;
	Vector3 modelScale;
	Vector4 colour;
	bool shadow;
	std::vector<SceneNode*> children;
	std::vector<GLuint> textures;
	std::vector<GLuint> bumpMaps;
	std::vector<GLuint> matTextures;
	float distanceFromCamera;
	float boundingRadius;
	GLuint texture;
	GLuint bumpMap;
	int currentFrame;
};