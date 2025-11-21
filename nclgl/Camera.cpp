#include "Camera.h"
#include "Window.h"
#include <algorithm>

void Camera::UpdateCamera(float dt) {
	pitch -= (Window::GetMouse()->GetRelativePosition().y);
	yaw -= (Window::GetMouse()->GetRelativePosition().x);

	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.0f);

	if (yaw < 0) {
		yaw += 360.0f;
	}
	if (yaw > 360) {
		yaw -= 360.0f;
	}

	Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));

	Vector3 forward = rotation * Vector3(0, 0, -1);
	Vector3 right = rotation * Vector3(1, 0, 0);

	float speed = 200.0f * dt;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
		position += forward * speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
		position -= forward * speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
		position -= right * speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
		position += right * speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
		position.y += speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
		position.y -= speed;

	}
}

Matrix4 Camera::BuildViewMatrix()
{
	return Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) * 
		Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) * 
		Matrix4::Translation(-position);
}

void Camera::AddCameraNode(const Vector3& positions, float yaws, float pitches) {
	railNodes.push_back({ positions, yaws, pitches });
}

void Camera::UpdateCameraRail(float dt, float speed)
{
	if (railNodes.empty()) return;

	const CameraNode& target = railNodes[currentNode];

	// Move position toward target
	Vector3 toTarget = target.position - position;
	float distance = toTarget.Length();

	if (distance < 0.1f) {
		// Arrived at this node and move to next
		currentNode++;
		if (currentNode >= railNodes.size())
			currentNode = 0; // loop (or remove to stop at end)
		return;
	}

	Vector3 direction = toTarget.Normalised();
	position += direction * speed * dt;

	// Smooth yaw/pitch interpolation
	yaw += (target.yaw - yaw) * (dt * 2.0f);
	pitch += (target.pitch - pitch) * (dt * 2.0f);
	std::cout << "pos: " << position << "  dist: " << distance << std::endl;
}
