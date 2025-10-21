#include "../nclgl/window.h"
#include "../nclgl/OGLRenderer.h"

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent) : OGLRenderer(parent) {
		triangle = Mesh::GenerateTriangle();

		basicShader = new Shader("basicVertex.glsl", "colourFragment.glsl");

		if (!basicShader->LoadSuccess()) {
			return;
		}
		init = true;
	}
	~Renderer(void) override {
		delete triangle;
		delete basicShader;
	}
	virtual void RenderScene() {
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		BindShader(basicShader);
		triangle->Draw();
	}

protected:
	Mesh* triangle;
	Shader* basicShader;
};

int main() {
	Window w("My First OpenGL Triangle!", 1280, 720, false);

	if (!w.HasInitialised()) {
		return -1;
	}

	Renderer renderer(w);
	if (!renderer.HasInitialised()) {
		return -1;
	}

	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		renderer.RenderScene();
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
			Shader::ReloadAllShaders();
		}
	}
	return 0;
}