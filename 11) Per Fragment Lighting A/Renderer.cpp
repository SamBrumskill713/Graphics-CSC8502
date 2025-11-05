#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	heightMap = new HeightMap(TEXTUREDIR"noise.png");
	texture = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	shader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");

	if (!shader->LoadSuccess() || !texture) {
		return;
	}

	SetTextureRepeating(texture, true);

	Vector3 heightampSize = heightMap->GetHeightmapSize();
}
