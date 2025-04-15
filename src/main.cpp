//#include "opengl.hh"
#include "libdisplayinit.h"
#include "RenderManager.h"

int main(int argc, char* argv[]) {

	printf("Q3 F3 Team OpenGL render 0.0.1 (QNX MOST VNC render 0.1.0 fork)\n");

	RenderManager renderManager(3, 800, 480);

	while(true) {
		renderManager.render();
	}

	/*
	printf("Loading config.txt \n");
    // Load config
    loadConfig("config.txt");

    // Print loaded or default values
    printArray("Landscape vertices", landscapeVertices, 12, 3);
    printArray("Portrait vertices", portraitVertices, 12, 3);
    printArray("Landscape texture coordinates", landscapeTexCoords, 8, 2);
    printArray("Portrait texture coordinates", portraitTexCoords, 8, 2);
    printf("windowWidth = %d;\n", windowWidth);
    printf("windowHeight = %d;\n", windowHeight);

    // Init Screen
    LibDisplayInit* libDisplayInit = LibDisplayInit::GetInstance();
    LibDisplayInit::dint_screen_window* dint_window = 0;
    int native_window = 0;

	int result = 0;

	// Call dint_init()
	result = libDisplayInit->dint_init();
	printf("dint_init result = %d\n", result);

	// Call dint_create_window()
	int display_id = 3;
	//int properties[] = {5, 1, 0};
	int *properties = 0;
	result = libDisplayInit->dint_create_window(0, display_id, windowWidth, windowHeight, properties, &dint_window);
	if(result) {
		printf("Failed to create window %d!!!\n", result);
		exit(EXIT_FAILURE);
	}
	printf("dint_create_window result = %d, native_window = %p, nBuffers = %d\n", result, dint_window->native_window, dint_window->nBuffers);

	// Call dint_get_native_window()
	result = libDisplayInit->dint_get_native_window(dint_window, (screen_window_t *)&native_window);
	if(result || !native_window) {
		printf("Failed to get native window %d!!!\n", result);
		exit(EXIT_FAILURE);
	}
	printf("dint_get_native_window result = %d, native_window = %p\n", result, native_window);
	// Init Screen Done

	// Init EGL
	printf("OpenGL ES2.0 initialization started \n");
	eglDisplay = libEgl->eglGetDisplay(EGL_DEFAULT_DISPLAY); // DONE
	if(eglDisplay == EGL_NO_DISPLAY) {
		printf("ERROR: no EGL Display\n");
		exit(EXIT_FAILURE);
	}
	printf("eglGetDisplay %p\n", eglDisplay);
	int major = 0, minor = 0;
	if(libEgl->eglInitialize(eglDisplay, &major, &minor) == EGL_FALSE) {
		printf("Error eglInitialize!!!\n");
		exit(EXIT_FAILURE);
	}
	printf("eglInitialize: Version %d.%d\n", major, minor);

	printf("OpenGLES: eglBindAPI \n");
	if(libEgl->eglBindAPI(EGL_OPENGL_ES_API) == EGL_FALSE) {
		printf("Error eglBindAPI!!!\n");
		checkErrorEGL("eglBindAPI");
		exit(EXIT_FAILURE);
	}

	EGLint config_attribs[] = {
	    EGL_RED_SIZE, 8,
	    EGL_GREEN_SIZE, 8,
	    EGL_BLUE_SIZE, 8,
	    EGL_DEPTH_SIZE, 16,
	    EGL_SAMPLE_BUFFERS, 0, // 1
	    EGL_SAMPLES, 0, // 4
	    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	    EGL_NONE
	};

	EGLint num_configs = 1;
	EGLSurface eglSurface = EGL_NO_SURFACE;
	EGLConfig eglConfig;

	libEgl->eglChooseConfig(eglDisplay, config_attribs, &eglConfig, num_configs, &num_configs);
	printf("eglChooseConfig: config = %p, num_configs = %d\n", eglConfig, num_configs);

	if(!num_configs || !eglConfig) {
		printf("Wrong egl config\n");
		return EXIT_FAILURE;
	}

	// config debug
	EGLint val;
	libEgl->eglGetConfigAttrib(eglDisplay, eglConfig, EGL_SURFACE_TYPE, &val);
	printf("  EGL_SURFACE_TYPE = 0x%x\n", val);
	if (!(val & EGL_WINDOW_BIT)) {
		printf("Config does not support EGL_WINDOW_BIT\n");
		return EXIT_FAILURE;
	}
	libEgl->eglGetConfigAttrib(eglDisplay, eglConfig, EGL_RENDERABLE_TYPE, &val);
	printf("  EGL_RENDERABLE_TYPE = 0x%x\n", val);
	libEgl->eglGetConfigAttrib(eglDisplay, eglConfig, EGL_RED_SIZE, &val);
	printf("  EGL_RED_SIZE = %d\n", val);
	libEgl->eglGetConfigAttrib(eglDisplay, eglConfig, EGL_GREEN_SIZE, &val);
	printf("  EGL_GREEN_SIZE = %d\n", val);
	libEgl->eglGetConfigAttrib(eglDisplay, eglConfig, EGL_BLUE_SIZE, &val);
	printf("  EGL_BLUE_SIZE = %d\n", val);

	const EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	printf("OpenGLES: eglCreateContext \n");
	eglContext = libEgl->eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, context_attribs);
	checkErrorEGL("eglCreateContext");
	if (eglContext == EGL_NO_CONTEXT) {
		std::cerr << "Failed to create EGL context" << std::endl;
		return EXIT_FAILURE;
	}

	eglSurface = libEgl->eglCreateWindowSurface(eglDisplay, eglConfig, (EGLNativeWindowType)native_window, 0);
	if(eglSurface == EGL_NO_SURFACE) {
		printf("Error: no eglSurface!!!\n");
		exit(EXIT_FAILURE);
	}

	printf("OpenGLES: eglMakeCurrent \n");
	EGLBoolean madeCurrent = libEgl->eglMakeCurrent(eglDisplay, eglSurface, eglSurface,	eglContext);
	if (madeCurrent == EGL_FALSE) {
		std::cerr << "Failed to make EGL context current" << std::endl;
		return EXIT_FAILURE;
	}

	printf("First part successful \n");


	// Initialize OpenGL ES
	Init();

	//test

	while(true) {
        libGl2->glClear(GL_COLOR_BUFFER_BIT);
        libGl2->glUseProgram(programObjectTextRender);
        print_string(-333, 160, "Q3 F3 Team OpenGL", 0.5, 0.5, 0.5, 150);
        libEgl->eglSwapBuffers(eglDisplay, eglSurface);
	}

	// Cleanup
	libEgl->eglSwapBuffers(eglDisplay, eglSurface);
	libEgl->eglDestroySurface(eglDisplay, eglSurface);
	libEgl->eglDestroyContext(eglDisplay, eglContext);
	libEgl->eglTerminate(eglDisplay);
	execute_final_commands();

	*/

	return 0;
}
