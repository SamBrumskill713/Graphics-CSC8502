#include "../NCLGL/window.h"
#include "../NCLGL/light.h"
#include "Renderer.h"

int main() {
	Light* light;
	Vector3 position(0.0f, 0.0f, -20.0f);
	Window w("Shadow Mapping!", 1280,720,false); //This is all boring win32 window creation stuff!
	if(!w.HasInitialised()) {
		return -1;
	}
	
	Renderer renderer(w); //This handles all the boring OGL 3.2 initialisation stuff, and sets up our tutorial!
	if(!renderer.HasInitialised()) {
		return -1;
	}

	w.LockMouseToWindow(true);
	w.ShowOSPointer(false);

	while(w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)){
		renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
		renderer.RenderScene();
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
			Shader::ReloadAllShaders();
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
			position.x -= 1.0f;
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
			position.x += 1.0f;	
		}
		//light->SetPosition(position);
	}

	return 0;
}