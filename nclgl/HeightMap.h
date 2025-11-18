#pragma once
#include <string>
#include "Mesh.h"

class HeightMap : public Mesh {
public:
	HeightMap(const std::string& name);
	~HeightMap(void) {}
	float HeightMap::GetHeightAt(float worldX, float worldZ) const;
	Vector3 GetHeightmapSize() const { return heightmapSize; }

protected:
	Vector3 heightmapSize;
	int height;
	int width;
};