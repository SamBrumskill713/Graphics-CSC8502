#include "HeightMap.h"
#include <iostream>

HeightMap::HeightMap(const std::string& name)
{
	int iWidth, iHeight, iChans;
	unsigned char* data = SOIL_load_image(name.c_str(), &iWidth, &iHeight, &iChans, 1);

	if (!data) {
		std::cout << "Heightmap can't load file!\n";
		return;
	}
	numVertices = iWidth * iHeight;
	numIndices = (iWidth - 1) * (iHeight - 1) * 6;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];

	Vector3 vertexScale = Vector3(16.0f, 1.0f, 16.0f);
	Vector2 textureScale = Vector2(1 / 16.0f, 1 / 16.0f);

	for (int z = 0; z < iHeight; ++z) {
		for (int x = 0; x < iWidth; ++x) {
			int offset = (z * iWidth) + x;
			vertices[offset] = Vector3(x, data[offset], z) * vertexScale;
			textureCoords[offset] = Vector2(x, z) * textureScale;
		}
	}
	SOIL_free_image_data(data);

	int i = 0;

	for (int z = 0; z < iHeight - 1; ++z) {
		for (int x = 0; x < iWidth - 1; ++x) {
			int a = (z * (iWidth)) + x;
			int b = (z * (iWidth)) + (x + 1);
			int c = ((z + 1) * (iWidth)) + (x + 1);
			int d = ((z + 1) * (iWidth)) + x;

			indices[i++] = a;
			indices[i++] = c;
			indices[i++] = b;

			indices[i++] = c;
			indices[i++] = a;
			indices[i++] = d;
		}
	}
	GenerateNormals();
	GenerateTangents();
	BufferData();

	heightmapSize.x = vertexScale.x * (iWidth - 1);
	heightmapSize.y = vertexScale.y * 255.0f;
	heightmapSize.z = vertexScale.z * (iHeight - 1);
	width = iWidth;
	height = iHeight;
}

float HeightMap::GetHeightAt(float worldX, float worldZ) const
{
	// Convert world coords to heightmap grid coords
	// Heightmap uses vertexScale.x and vertexScale.z for spacing.
	float gridX = worldX / 16.0f;   // vertexScale.x
	float gridZ = worldZ / 16.0f;   // vertexScale.z

	// Compute integer cell position
	int xInt = (int)floor(gridX);
	int zInt = (int)floor(gridZ);

	// Clamp / boundary check
	if (xInt < 0 || zInt < 0) return 0.0f;
	if (xInt >= width - 1 || zInt >= height - 1) return 0.0f;

	// Compute interpolation fractions inside the cell
	float fracX = gridX - xInt;
	float fracZ = gridZ - zInt;

	// Lookup heightmap height values (raw image byte was stored in Y)
	float h00 = vertices[(zInt * width) + xInt].y;
	float h10 = vertices[(zInt * width) + (xInt + 1)].y;
	float h01 = vertices[((zInt + 1) * width) + xInt].y;
	float h11 = vertices[((zInt + 1) * width) + (xInt + 1)].y;

	// Bilinear interpolation
	float h0 = (h10 - h00) * fracX + h00;
	float h1 = (h11 - h01) * fracX + h01;
	float h = (h1 - h0) * fracZ + h0;

	return h;
}
