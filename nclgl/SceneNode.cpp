#include "SceneNode.h"

SceneNode::SceneNode()
{
	this->mesh = nullptr;
	this->heightMap = nullptr;
	this->colour = Vector4(1, 1, 1, 1);
	shader = nullptr;
	parent = nullptr;
	heightMap = nullptr;
	modelScale = Vector3(1, 1, 1);
	boundingRadius = 5000.0f;
	distanceFromCamera = 0.0f;
	texture = 0;
}

SceneNode::SceneNode(Mesh* m, Vector4 colour)
{
	this->mesh = m;
	this->colour = colour;
	shader = nullptr;
	parent = nullptr;
	heightMap = nullptr;
	modelScale = Vector3(1, 1, 1);
	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;
	texture = 0;
}

SceneNode::SceneNode(HeightMap* heightmap, Vector4 colour)
{
	heightMap = heightmap;
	this->colour = colour;
	parent = nullptr;
	shader = nullptr;
	mesh = nullptr;
	modelScale = Vector3(1, 1, 1);
	boundingRadius = 5000.0f;
	distanceFromCamera = 0.0f;
	texture = 0;
}

SceneNode::~SceneNode(void)
{
	for (unsigned int i = 0; i < children.size(); ++i) {
		delete children[i];
	}
}

void SceneNode::setShaderTextures()
{
	if (shader) {
		if (heightMap) {
			if (textures.size() >= 2) {
				glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textures[0]);

				glUniform1i(glGetUniformLocation(shader->GetProgram(), "bumpTex"), 1);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, textures[1]);
			}
			return;
		}

		else if (mesh && mesh->GetSubMeshCount() == 1)
		{
			if (!textures.empty())
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textures[0]);
				glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
			}
			return;
		}

		else if (mesh && mesh->GetSubMeshCount() > 1)
		{
			for (int i = 0; i < matTextures.size(); ++i)
			{
				std::string uniformName = "diffuseTex" + std::to_string(i);
				GLuint tex = matTextures[i];

				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, tex);

				glUniform1i(
					glGetUniformLocation(shader->GetProgram(), uniformName.c_str()),i);
			}
		}
	}
}

void SceneNode::AddChild(SceneNode* s)
{
	children.push_back(s);
	s->parent = this;
}

void SceneNode::RemoveChild(int index)
{
	children.erase(children.begin() + index);
}

void SceneNode::Update(float dt)
{
	if (parent) { worldTransform = parent->worldTransform * transform; }
	else { worldTransform = transform; }
	for (vector<SceneNode*>::iterator i = children.begin(); i != children.end(); ++i) {
		(*i)->Update(dt);
	}
}

void SceneNode::Draw(const OGLRenderer& r)
{
	if (mesh) { mesh->Draw(); }
	if (heightMap) {
		heightMap->Draw();
	}
}