#pragma once
#include "vector3.h"

class Plane {
public:
	Plane(void) {};
	Plane(const Vector3& normal, float distance, bool normalised = false);
	~Plane(void) {};

	void SetNormal(const Vector3)
};