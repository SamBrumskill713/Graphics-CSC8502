#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector2.h"
#include <vector>

class Camera {
public:
	struct CameraNode {
		Vector3 Positions;
		float railYaw;
		float railPitch;
	};

	Camera(void) {
		yaw = 0.0f;
		pitch = 0.0f;
		position = Vector3(0.0f, 0.0f, 0.0f);
	};

	Camera(float pitch, float yaw, Vector3 position) {
		this->pitch = pitch;
		this->yaw = yaw;
		this->position = position;
	}

	~Camera(void) {};

	void UpdateCamera(float dt = 1.0f);
	
	Matrix4 BuildViewMatrix();

	Vector3 GetPosition() const { return position; }
	void SetPosition(Vector3 val) { position = val; }

	float GetYaw() const { return yaw; }
	void SetYaw(float y) { yaw = y; }

	float GetPitch() const { return pitch; }
	void SetPitch(float p) { pitch = p; }

protected:
	bool isCameraRail = false;
	float yaw;
	float pitch;
	Vector3 position;
	int currentNode = 0;
	float speed = 1.0f;
	std::vector<Vector3> positionVec;
	std::vector<float> pitchVec;
	std::vector<float> yawVec;
};