#include "../NCLGL/window.h"
#include "Renderer.h"
#include <iostream>

int main()	{
	float cooldown = 0.25;
	Window w("CSC5802", 1280, 720, false);

	if(!w.HasInitialised()) {
		return -1;
	}
	
	Renderer renderer(w);
	if(!renderer.HasInitialised()) {
		return -1;
	}

	w.LockMouseToWindow(true);
	w.ShowOSPointer(false);

	while(w.UpdateWindow()  && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)){
		renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
		renderer.RenderScene();
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
			Shader::ReloadAllShaders();
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_1) && cooldown <= 0) {
			cooldown = 0.25;
			renderer.toggleCamera();
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F) && cooldown <= 0) {
			cooldown = 0.25;
			renderer.toggleScene();
		}
		cooldown -= w.GetTimer()->GetTimeDeltaSeconds();
	}
	return 0;
}