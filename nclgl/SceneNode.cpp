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
	boundingRadius = 1.0f;
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

		if (mesh && getMatTextureSize() == 1) {
			if (!matTextures.empty()) {
				glUniform1i(glGetUniformLocation(GetShader()->GetProgram(),"useTexture"), 
					GetMatTexture(0));
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, matTextures[0]);
			}
			return;
		}

		else if (mesh && mesh->GetSubMeshCount() == 1)
		{
			if (!textures.empty())
			{
				glUniform1i(glGetUniformLocation(GetShader()->GetProgram(), "useTexture"), 
					0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textures[0]);
			}
			return;
		}

		else if (mesh && mesh->GetSubMeshCount() > 1)
		{
			if (!matTextures.empty()) {
				for (int i = 0; i < GetMesh()->GetSubMeshCount(); ++i) {
					glUniform1i(glGetUniformLocation(GetShader()->GetProgram(),
						"useTexture"), matTextures[i]);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, matTextures[i]);

					GetMesh()->DrawSubMesh(i);
				}
			}
			return;
		}
		
		else if (mesh && GetAnimation()) {
			if (!GetAnimation()) {
				glUniform1i(glGetUniformLocation(GetShader()->GetProgram(),
					"diffuseTex"), 0);

				vector<Matrix4> frameMatrices;

				const Matrix4* invBindPose = GetMesh()->GetInverseBindPose();
				const Matrix4* frameData = GetAnimation()->GetJointData(currentFrame);

				for (unsigned int i = 0; i < GetMesh()->GetJointCount(); ++i) {
					frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
				}

				int j = glGetUniformLocation(GetShader()->GetProgram(), "joints");
				glUniformMatrix4fv(j, frameMatrices.size(), false, 
					(float*)frameMatrices.data());

				for (int i = 0; i < GetMesh()->GetSubMeshCount(); ++i) {
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, GetMatTexture(i));
					GetMesh()->DrawSubMesh(i);
				}
			}
			return;
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
	if (mesh) { 
		mesh->Draw(); 
	}
	if (heightMap) {
		heightMap->Draw();
	}
}