#include "Renderer.h"
#include "../nclgl/camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Light.h"
#include "../nclgl/Mesh.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include "../nclgl/Matrix4.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustrum.h"
#include <iostream>
#include <vector>
#include <algorithm>

#define SHADOWSIZE 2048
const int UFO_NUM = 4;
const int LIGHT_NUM = 25;
const int TREE_NUM = 10;

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	checkMeshes();
	checkModelMatrial();
	checkAnimation();
	checkTextures();
	checkShaders();
	setVariables();
	GenerateScreenTexture(bufferDepthTex, true);
	GenerateScreenTexture(bufferColourTex);
	GenerateScreenTexture(bufferNormalTex);
	GenerateScreenTexture(lightDiffuseTex);
	GenerateScreenTexture(lightSpecularTex);
	checkBuffers();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	init = true;
}

Renderer::~Renderer(void) {
	delete heightMap;
	delete activeCamera;
	//delete freeCamera;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete characterShader;
	delete light;
	delete soldier;
	delete soldierAnimation;
	delete soldierMaterial;
	delete shadowShader;
	delete sceneShader;
	delete pointLightShader;
	delete combineShader;
	delete nodeShader;
	delete characterShadowShader;
	delete quad;
	delete sphere;
	delete[] pointLights;
	delete landMapRoot;
	delete Tree;
	delete TreeMaterial;
	delete UFO;
	delete UFOMaterial;
	delete awesomeSkeleton;
	delete awesomeSkeletonMaterial;
	delete awesomeSkeletonAnimation;
	delete houseMesh;
	delete houseMaterial;
	delete heightMap;
	delete heightMap2;
	for (auto tex : awesomeSkeletonMatTextures) { glDeleteTextures(1, &tex); }
	for (auto tex : houseMatTextures) { glDeleteTextures(1, &tex); }
	for (auto tex : soldierMatTextures) { glDeleteTextures(1, &tex); }
	for (auto tex : TreeMatTextures) { glDeleteTextures(1, &tex); }
	for (auto tex : UFOMatTextures) { glDeleteTextures(1, &tex); }
	if (terrainTex) { glDeleteTextures(1, &terrainTex); }
	if (waterTex) { glDeleteTextures(1, &waterTex); }
	if (terrainBump) { glDeleteTextures(1, &terrainBump); }
	if (cubeMap) { glDeleteTextures(1, &cubeMap); }
	glDeleteTextures(1, &bufferColourTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &lightDiffuseTex);
	glDeleteTextures(1, &lightSpecularTex);
	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &pointLightFBO);
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
}

void Renderer::UpdateScene(float dt) {
	activeCamera->UpdateCamera(dt);
	viewMatrix = activeCamera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f,
		(float)width / (float)height, 45.0f);
	frameFrustrum.FromMatrix(projMatrix * viewMatrix);
	if (isMainScene && landMapRoot && !isTransScene) {
		landMapRoot->Update(dt);
	}
	else if (isTransScene && transSceneRoot && !isMainScene) {
		transSceneRoot->Update(dt);
	}
	waterRotate += dt;
	waterCycle += dt;
	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % soldierAnimation->GetFrameCount();
		frameTime += 1.0f / soldierAnimation->GetFrameRate();
	}
	soldierModel = Matrix4::Translation(Vector3(0, heightMap->GetHeightAt(soldierPos.x,
		soldierPos.z + (direction * 50 * dt)) - soldierPos.y, direction * 50 * dt)) *
		soldierModel;
	soldierPos = soldierPos + Vector3(0, 0, direction * 50 * dt);
	soldierPos.y = heightMap->GetHeightAt(soldierPos.x, soldierPos.z);
	if (soldierPos.y < 85) {
		soldierModel = Matrix4::Translation(soldierPos) * Matrix4::Rotation(180,
			Vector3(0, 1, 0)) * Matrix4::Translation(-soldierPos) * soldierModel;
		direction *= -1;
	}
	if (cubeNode != nullptr) {
		cubeAngle += cubeRotateSpeed * dt;
		if (cubeAngle >= 360.0f) cubeAngle -= 360.0f;

		cubeNode->SetTransform(Matrix4::Translation(cubePos) *
			Matrix4::Rotation(cubeAngle, Vector3(70, 60, 50)) *
			Matrix4::Scale(Vector3(500, 500, 500)));
	}
	checkCurrentCamera();
}

void Renderer::RenderScene() {
	modelMatrix.ToIdentity();
	viewMatrix = activeCamera->BuildViewMatrix();
	if (isMainScene && landMapRoot && !isTransScene) {
		buildNodeLists(landMapRoot);
	}
	else if (isTransScene && transSceneRoot && !isMainScene) {
		buildNodeLists(transSceneRoot);
	}
	sortNodeList();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkybox();
	DrawShadowScene(&pointLights[0]);
	fillBuffers();
	createPointLights();
	combineBuffers();
	//drawNodes();
}

void Renderer::DrawWater(float transparancy)
{
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1,
		(float*)&activeCamera->GetPosition());
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);
	glUniform1f(glGetUniformLocation(reflectShader->GetProgram(), "transparancy"), 
		transparancy);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightmapSize();
	Vector3 hSize2 = heightMap2->GetHeightmapSize();	

	if (isMainScene && !isTransScene) {
		modelMatrix =
			Matrix4::Translation(hSize * 0.5f) *
			Matrix4::Scale(hSize * 0.5f) *
			Matrix4::Rotation(90, Vector3(1, 0, 0));

		textureMatrix =
			Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
			Matrix4::Scale(Vector3(10, 10, 10)) *
			Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));
	}

	if (isTransScene && !isMainScene) {
		modelMatrix =
			Matrix4::Translation(hSize2 * 0.5f) *
			Matrix4::Scale(hSize2 * 0.5f) *
			Matrix4::Rotation(90, Vector3(1, 0, 0));

		textureMatrix =
			Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
			Matrix4::Scale(Vector3(10, 10, 10)) *
			Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));
	}

	UpdateShaderMatrices();
	SetShaderLight(*light);
	quad->Draw();
}

void Renderer::DrawSkybox() 
{
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

void Renderer::checkTextures()
{
	terrainTex = SOIL_load_OGL_texture(TEXTUREDIR"Swampland.jpg", 
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	
	terrainBump = SOIL_load_OGL_texture(TEXTUREDIR"SwamplandDOT3.jpg", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	cubeTexture = SOIL_load_OGL_texture(TEXTUREDIR"evil.png", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"right.png", TEXTUREDIR"left.png",
		TEXTUREDIR"top.png", TEXTUREDIR"bottom.png",
		TEXTUREDIR"front.png", TEXTUREDIR"back.png",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	for (int i = 0; i < soldier->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = soldierMaterial->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(),
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		soldierMatTextures.emplace_back(texID);
	}

	for (int i = 0; i < Tree->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = TreeMaterial->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(),
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		TreeMatTextures.emplace_back(texID);
	}

	for (int i = 0; i < UFO->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = UFOMaterial->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(),
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		UFOMatTextures.emplace_back(texID);
	}

	for (int i = 0; i < awesomeSkeleton->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = awesomeSkeletonMaterial->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(),
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		awesomeSkeletonMatTextures.emplace_back(texID);
	}

	for (int i = 0; i < houseMesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = houseMaterial->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(),
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		houseMatTextures.emplace_back(texID);
	}

	if (!terrainTex || !waterTex || !terrainBump || !cubeMap || !cubeTexture) {
		return;
	}

	SetTextureRepeating(terrainTex, true);
	SetTextureRepeating(terrainBump, true);
	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(cubeTexture, true);
}

void Renderer::checkShaders()
{
	//shader = new Shader("TexturedVertex.glsl", "texturedfragment.glsl");
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");
	characterShader = new Shader("SkinningVertex.glsl", "texturedfragment.glsl");
	shadowShader = new Shader("shadowVertex.glsl", "shadowFragment.glsl");
	sceneShader = new Shader("BumpVertex.glsl", "bufferFragment.glsl");
	nodeShader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
	pointLightShader = new Shader("pointLightvert.glsl", "pointLightFrag.glsl");
	combineShader = new Shader("combineVert.glsl", "combineFrag.glsl");
	characterShadowShader = new Shader("shadowSkinning.glsl", "shadowFragment.glsl");
	processShader = new Shader("TexturedVertex.glsl", "processfrag.glsl");

	if (!reflectShader->LoadSuccess() ||
		!skyboxShader->LoadSuccess() ||
		!lightShader->LoadSuccess() ||
		!characterShader->LoadSuccess() ||
		!shadowShader->LoadSuccess() ||
		!sceneShader->LoadSuccess() ||
		!pointLightShader->LoadSuccess() ||
		!combineShader->LoadSuccess()) {
		return;
	}
}

void Renderer::checkBuffers()
{
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE,
		SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		shadowTex, 0);
	glDrawBuffer(GL_NONE);

	//glGenFramebuffers(1, )

	GLenum buffers[2] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &pointLightFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		bufferColourTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
		bufferNormalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		bufferDepthTex, 0);
	glDrawBuffers(2, buffers);

	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		lightDiffuseTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
		lightSpecularTex, 0);
	glDrawBuffers(2, buffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::setCameraNodes()
{
}

void Renderer::setVariables()
{
	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	Vector3 heightmapSize2 = heightMap2->GetHeightmapSize();
	mainSceneCamera = new Camera(-30, 180, Vector3(heightmapSize.x/2, 2000, 
		heightmapSize.z));
	activeCamera = mainSceneCamera;
	transSceneCamera = new Camera(-30, 90, 
		Vector3(heightmapSize2.x / 2 + 800, 500, heightmapSize2.z / 4 - 1000));
	isCameraFree = false;
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, 
		(float)width / (float)height, 45.0f);
	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f),
		Vector4(1, 1, 1, 1), heightmapSize.x);
	pointLights = new Light[LIGHT_NUM];
	soldierPos = Vector3(heightmapSize.x / 2, 
		heightMap->GetHeightAt(heightmapSize.x/2, heightmapSize.z/2), heightmapSize.z / 2);
	soldierModel = Matrix4::Translation(soldierPos) * 
		Matrix4::Rotation(180, Vector3(0, 1, 0)) * 
		Matrix4::Scale(Vector3(100, 100, 100));
	TreePos = Vector3(rand() % (int)heightmapSize.x, 0, rand() % (int)heightmapSize.z);
	TreeModel = Matrix4::Translation(TreePos) *
		Matrix4::Scale(Vector3(50, 50, 50));
	UFOPos = Vector3(heightmapSize.x / 2, heightmapSize.y, heightmapSize.z / 2);
	UFOModel = Matrix4::Translation(UFOPos) *
		Matrix4::Scale(Vector3(20, 20, 20));
		Matrix4::Scale(Vector3(20, 20, 20));
	skelPos = Vector3(2000,
		heightMap->GetHeightAt(2000, 2000), 2000);
	skelModel = Matrix4::Translation(skelPos) *
		Matrix4::Scale(Vector3(200.0f, 200.0f, 200.0f));
	cubePos = Vector3(heightmapSize.x / 2 - 1300, 1000, heightmapSize.z / 2 - 1500);
	cubeModel = Matrix4::Translation(cubePos) *
		Matrix4::Scale(Vector3(500.0f, 500.0f, 500.0f));
	pointLights = new Light[LIGHT_NUM];
	Light& l = pointLights[0];
	l.SetPosition(Vector3(1000.0f, 1000.0f, -2000.0f));
	l.SetColour(Vector4(0.95f, 0.9f, 0.85f, 1));
	l.SetRadius(2000);

	for (int i = 1; i < LIGHT_NUM; ++i) {
		pointLights[i].SetPosition(Vector3(rand() % (int)heightmapSize.x, 150.0f,
			rand() % (int)heightmapSize.z));
		pointLights[i].SetColour(Vector4(0.95f, 0.9f, 0.85f, 1));
		pointLights[i].SetRadius(2000);
	}
	landMapRoot = new SceneNode();
	transSceneRoot = new SceneNode();
	setNodes();
	waterRotate = 0.0f;
	waterCycle = 0.0f;
	currentFrame = 0;
	frameTime = 0.0f;
	//isShadow = false;
}

void Renderer::setNodes() {
	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	SceneNode* heightmapNode = new SceneNode(heightMap);
	heightmapNode->SetShader(sceneShader);
	heightmapNode->AddTexture(terrainTex);
	heightmapNode->AddTexture(terrainBump);
	heightmapNode->SetBoundingRadius(heightmapSize.Length());
	landMapRoot->AddChild(heightmapNode);

	for (int i = 0; i < TREE_NUM; ++i) {
		SceneNode* TreeNode = new SceneNode(Tree);
		TreeNode->SetShader(nodeShader);
		int randX = rand() / (RAND_MAX / 8176);
		int randZ = rand() / (RAND_MAX / 8176);
		TreeNode->SetTransform(Matrix4::Translation(Vector3(randX,
			heightMap->GetHeightAt(randX, randZ), randZ)));
		TreeNode->SetMatTextures(TreeMatTextures);
		TreeNode->SetModelScale(Vector3(100.0f, 100.0f, 100.0f));
		TreeNode->SetBoundingRadius(700.0f);
		heightmapNode->AddChild(TreeNode);
	}

	SceneNode* UFONode = new SceneNode(UFO);
	UFONode->SetShader(nodeShader);
	UFONode->SetTransform(UFOModel);
	UFONode->SetMatTextures(UFOMatTextures);
	UFONode->SetBoundingRadius(200.0f);
	//UFOBodyNode->SetModelScale();
	heightmapNode->AddChild(UFONode);
	
	Vector3 heightmapSize2 = heightMap2->GetHeightmapSize();
	SceneNode* transHeightmapNode = new SceneNode(heightMap2);
	transHeightmapNode->SetShader(sceneShader);
	transHeightmapNode->AddTexture(terrainTex);
	transHeightmapNode->AddTexture(terrainBump);
	transHeightmapNode->SetBoundingRadius(heightmapSize.Length());
	transSceneRoot->AddChild(transHeightmapNode);

	SceneNode* houseNode = new SceneNode(houseMesh);
	houseNode->SetShader(nodeShader);
	houseNode->SetTransform(Matrix4::Translation(Vector3(4000,
		heightMap->GetHeightAt(4000, 4000), 4000)) * Matrix4::Scale(Vector3(100.0f,
			100.0f, 100.0f)));
	houseNode->SetMatTextures(houseMatTextures);
	houseNode->SetBoundingRadius(500.0f);
	transHeightmapNode->AddChild(houseNode);

	SceneNode* cubeNodeLocal = new SceneNode(cube);
	cubeNodeLocal->SetShader(nodeShader);
	cubeNodeLocal->SetTransform(cubeModel);
	cubeNodeLocal->SetBoundingRadius(700.0f);
	cubeNodeLocal->AddTexture(cubeTexture);
	cubeNode = cubeNodeLocal;
	transHeightmapNode->AddChild(cubeNodeLocal);
}

void Renderer::buildNodeLists(SceneNode* from)
{

	Vector3 pos = from->GetWorldTransform().GetPositionVector();
	float radius = from->GetBoundingRadius();

	bool inside = frameFrustrum.InsideFrustrum(*from);
	if (frameFrustrum.InsideFrustrum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - 
			activeCamera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			nodeList.push_back(from);
		}
	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); 
		i != from->GetChildIteratorEnd(); ++i) {
		buildNodeLists((*i));
	}
}

void Renderer::sortNodeList()
{
	std::sort(transparentNodeList.rbegin(), transparentNodeList.rend(), 
		SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
}

void Renderer::drawNodes()
{
	for (const auto& i : nodeList) {
		drawNode(i);
	}
	for (const auto& i : transparentNodeList) {
		drawNode(i);
	}
}

void Renderer::clearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}

void Renderer::drawNode(SceneNode* n)
{
	if (isShadow) {
		modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		UpdateShaderMatrices();
		n->Draw(*this);
	}

	else if (!isShadow) {
		Shader* nodeShader = n->GetShader();
		if (nodeShader != nullptr) {
			BindShader(nodeShader);
		}
		UpdateShaderMatrices();
		if (n->GetMesh()) {
			Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
			if (nodeShader) {
				glUniformMatrix4fv(glGetUniformLocation(nodeShader->GetProgram(),
					"modelMatrix"), 1, false, model.values);
				glUniform4fv(glGetUniformLocation(nodeShader->GetProgram(),
					"nodeColour"), 1, (float*)&n->GetColour());
				n->setShaderTextures();
			}
			n->Draw(*this);
		}
		if (n->GetHeightMap() != nullptr) {
			if (nodeShader) {
				BindShader(nodeShader);
				n->setShaderTextures();
			}
			UpdateShaderMatrices();
			n->Draw(*this);
		}
	}
}

void Renderer::checkMeshes() {
	quad = Mesh::GenerateQuad();
	heightMap = new HeightMap(TEXTUREDIR"swampHeightmap.png");
	heightMap2 = new HeightMap(TEXTUREDIR"testspacetexture.png");
	soldier = Mesh::LoadFromMeshFile("Role_T.msh");
	sphere = Mesh::LoadFromMeshFile("Sphere.msh");
	Tree = Mesh::LoadFromMeshFile("DeadTree_1.msh");
	UFO = Mesh::LoadFromMeshFile("Low_poly_UFO.msh");
	houseMesh = Mesh::LoadFromMeshFile("house.msh");
	awesomeSkeleton = Mesh::LoadFromMeshFile("Skeleton@skin.msh");
	cube = Mesh::LoadFromMeshFile("cube.msh");
	if (heightMap == nullptr) {
		std::cout << "No model found";
		return;
	}
}

void Renderer::checkModelMatrial() {
	soldierMaterial = new MeshMaterial("Role_T.mat");
	TreeMaterial = new MeshMaterial("DeadTree_1.mat");
	UFOMaterial = new MeshMaterial("Low_poly_UFO.mat");
	houseMaterial = new MeshMaterial("house.mat");
	awesomeSkeletonMaterial = new MeshMaterial("Skeleton@skin.mat");
	if (soldierMaterial == nullptr || TreeMaterial == nullptr 
		|| UFOMaterial == nullptr) {
		std::cout << "No material found \n";
		return;
	}
}

void Renderer::checkAnimation() {
	soldierAnimation = new MeshAnimation("Role_T.anm");
	awesomeSkeletonAnimation = new MeshAnimation("Idle.anm");
	if (soldierAnimation == nullptr || awesomeSkeletonAnimation == nullptr) {
		std::cout << "No animation found \n";
		return;
	}
}

void Renderer::DrawAnimations() {
	Vector3 heightmapSize2 = heightMap2->GetHeightmapSize();
	if (!isMainScene && isTransScene) {
		soldierPos = Vector3(heightmapSize2.x / 2 + 800,
			heightMap->GetHeightAt(heightmapSize2.x/2, 1000), 1500);
		soldierModel = Matrix4::Translation(soldierPos) *
			Matrix4::Scale(Vector3(100.0f, 100.0f, 100.0f));
	}
	if (isShadow) {
		BindShader(characterShadowShader);
		modelMatrix = soldierModel;
		UpdateShaderMatrices();
		vector<Matrix4> frameMatrices;

		const Matrix4* invBindPose = soldier->GetInverseBindPose();
		const Matrix4* frameData = soldierAnimation->GetJointData(currentFrame);

		for (unsigned int i = 0; i < soldier->GetJointCount(); ++i) {
			frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
		}
		int j = glGetUniformLocation(characterShadowShader->GetProgram(), "joints");
		glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());
		for (int i = 0; i < soldier->GetSubMeshCount(); ++i) {
			soldier->DrawSubMesh(i);
		}
	}
	else if (!isShadow) {
		BindShader(characterShader);
		glUniform1i(glGetUniformLocation(characterShader->GetProgram(), "diffuseTex"), 0);

		modelMatrix = soldierModel;

		UpdateShaderMatrices();

		vector<Matrix4> frameMatrices;

		const Matrix4* invBindPose = soldier->GetInverseBindPose();
		const Matrix4* frameData = soldierAnimation->GetJointData(currentFrame);

		for (unsigned int i = 0; i < soldier->GetJointCount(); ++i) {
			frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
		}

		int j = glGetUniformLocation(characterShader->GetProgram(), "joints");
		glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

		for (int i = 0; i < soldier->GetSubMeshCount(); ++i) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, soldierMatTextures[i]);
			soldier->DrawSubMesh(i);
		}
	}
}

void Renderer::DrawShadowScene(Light* l)
{
	Vector3 heightMapSize = heightMap->GetHeightmapSize();
	isShadow = true;
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glCullFace(GL_FRONT);

	BindShader(shadowShader);

	viewMatrix = Matrix4::BuildViewMatrix(l->GetPosition(), Vector3(heightMapSize.x / 2,
		0, heightMapSize.z / 2));

	projMatrix = Matrix4::Perspective(100, 10000.0f, 1, 45);
	shadowMatrix = projMatrix * viewMatrix;

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();
	UpdateShaderMatrices();

	drawNodes();
	DrawAnimations();

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK);
	isShadow = false;
}

void Renderer::checkCurrentCamera()
{
	if (isCameraFree) {
		//activeCamera = freeCamera;
	}
}

void Renderer::GenerateScreenTexture(GLuint& into, bool depth)
{
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint format = depth ? GL_DEPTH_COMPONENT24 : GL_RGBA8;
	GLuint type = depth ? GL_DEPTH_COMPONENT : GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, type,
		GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::fillBuffers()
{
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(sceneShader);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "bumpTex"), 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainTex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, terrainBump);

	modelMatrix.ToIdentity();
	viewMatrix = activeCamera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 
		45.0f);

	UpdateShaderMatrices();
	drawNodes();
	clearNodeLists();
	DrawWater(0.1f);
	DrawAnimations();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::combineBuffers()
{
	BindShader(combineShader);
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "diffuseLight"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lightDiffuseTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "specularLights"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "depthTex"), 3);	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	quad->Draw();
	/*glClear(GL_DEPTH_BUFFER_BIT);*/
}

void Renderer::createPointLights()
{
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	BindShader(pointLightShader);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);

	glUniform1i(glGetUniformLocation(pointLightShader->GetProgram(), "depthTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glUniform1i(glGetUniformLocation(pointLightShader->GetProgram(), "normTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform1i(glGetUniformLocation(pointLightShader->GetProgram(), "shadowTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	GLint loc = glGetUniformLocation(pointLightShader->GetProgram(), "shadowMatrix");
	if (loc != -1) {
		glUniformMatrix4fv(loc, 1, false, shadowMatrix.values);
	}

	glUniform3fv(glGetUniformLocation(pointLightShader->GetProgram(), "cameraPos"),
		1, (float*)&activeCamera->GetPosition());

	glUniform2f(glGetUniformLocation(pointLightShader->GetProgram(), "pixelSize"),
		1.0f / width, 1.0f / height);

	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(pointLightShader->GetProgram(),
		"inverseProjView"), 1, false, invViewProj.values);
	UpdateShaderMatrices();
	for (int i = 0; i < LIGHT_NUM; ++i) {
		Light& l = pointLights[i];
		SetShaderLight(l);
		sphere->Draw();
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	glDepthMask(GL_TRUE);

	glClearColor(0.2f, 0.2f, 0.2f, 1);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}