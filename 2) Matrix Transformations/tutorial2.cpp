#include "../nclgl/window.h"
#include "../nclgl/OGLRenderer.h"

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent) : OGLRenderer(parent) {
		triangle = Mesh::GenerateTriangle();
		matrixShader = new Shader("MatrixVertex.glsl", "colourFragment.glsl");

		if (!matrixShader->LoadSuccess()) {
			return;
		}

		init = true;

		SwitchToPerspective();
	}
	virtual ~Renderer(void) {
		delete triangle;
		delete matrixShader;
	}

	virtual void RenderScene() {
		glClear(GL_COLOR_BUFFER_BIT);

		BindShader(matrixShader);

		glUniformMatrix4fv(glGetUniformLocation(matrixShader->GetProgram(), "projMatrix"), 1, false, projMatrix.values);
		glUniformMatrix4fv(glGetUniformLocation(matrixShader->GetProgram(), "viewMatrix"), 1, false, viewMatrix.values);

		for (int i = 0; i < 3; ++i) {
			Vector3 tempPos = position;
			tempPos.z += (i * 500.0f);
			tempPos.x += (i * 100.0f);
			tempPos.y += (i * 100.0f);

			modelMatrix = Matrix4::Translation(tempPos) *  Matrix4::Scale(Vector3(scale, scale, scale))* Matrix4::Rotation(rotation, Vector3(0, 1, 0));

			glUniformMatrix4fv(glGetUniformLocation(matrixShader->GetProgram(), "modelMatrix"), 1, false, modelMatrix.values);
			triangle->Draw();
		}
	};

	void SwitchToPerspective() {
		projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	}
	void SwitchToOrthographic() {
		projMatrix = Matrix4::Orthographic(-1.0, 10000.0f, width / 2.0, -width / 2.0, height / 2.0, -height / 2.0);
	}

	inline void SetScale(float s) { scale = s; }
	inline void SetRotation(float r) { rotation = r; }
	inline void SetPosition(Vector3 p) { position = p; }
	inline void setFOV(float f) { fov = f; }

protected:
	Mesh* triangle;
	Shader* matrixShader;
	float scale;
	float rotation;
	Vector3 position;
	float fov;
}; 

int main() {
	Window w("Vertex Transformation!", 800, 600, false);
	if (!w.HasInitialised()) {
		return -1;
	}

	Renderer renderer(w);
	if (!renderer.HasInitialised()) {
		return -1;
	}

	float scale = 100.0f;
	float rotation = 0.0f;
	float fov = 45.0f;
	Vector3 position(0, 0, -1500.0f);

	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_1))
			renderer.SwitchToOrthographic();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_2))
			renderer.SwitchToPerspective();

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_PLUS))  ++scale;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_MINUS)) --scale;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT))  ++rotation;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT)) --rotation;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_K))
			position.y -= 1.0f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_I))
			position.y += 1.0f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_J))
			position.x -= 1.0f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_L))
			position.x += 1.0f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_O))
			position.z -= 1.0f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_P))
			position.z += 1.0f;

		renderer.SetRotation(rotation);
		renderer.SetScale(scale);
		renderer.SetPosition(position);
		renderer.RenderScene();
		renderer.SwapBuffers();
	}

	return 0;
}