#include "RenderManager.h"
#include <stdexcept>
#include "libegl.h"
#include "libgl2.h"
#include "stb_easyfont.hh"
#include "wstring_to_utf8.hh"

RenderManager::RenderManager(int ctx, int w, int h)
	: context(ctx), width(w), height(h)
{
	printf("RenderManager::RenderManager(): context = %d, width = %d, height=%d\n", context, width, height);

	this->dint_window = 0;
	this->native_window = 0;
	this->eglDisplay = 0;
	this->eglSurface = 0;
	this->eglContext = 0;
	this->eglConfig = 0;
	this->programObjectTextRender = 0;

	this->init();
	this->createGlPrograms();
}

RenderManager::~RenderManager()
{
	printf("RenderManager::~RenderManager()\n");

	LibDisplayInit* libDisplayInit = LibDisplayInit::GetInstance();
    LibEgl* libEgl = LibEgl::GetInstance();
    LibGl2* libGl2 = LibGl2::GetInstance();

    if (eglContext != EGL_NO_CONTEXT) {
        libEgl->eglDestroyContext(eglDisplay, eglContext);
    }
    if (eglSurface != EGL_NO_SURFACE) {
        libEgl->eglDestroySurface(eglDisplay, eglSurface);
    }
    if (eglDisplay != EGL_NO_DISPLAY) {
        libEgl->eglTerminate(eglDisplay);
    }
    if (programObjectTextRender) {
        libGl2->glDeleteProgram(programObjectTextRender);
    }

    if(dint_window) {
    	libDisplayInit->dint_destroy_window(dint_window);
    }
    libDisplayInit->dint_deinit();
}

void RenderManager::init()
{
	LibDisplayInit* libDisplayInit = LibDisplayInit::GetInstance();
	LibEgl* libEgl = LibEgl::GetInstance();

	int result = 0;

	// Call dint_init()
	result = libDisplayInit->dint_init();
	if(result) {
		printf("ERROR: dint_init(): %d\n", result);
		throw std::runtime_error("dint_init");
	}

	this->eglDisplay = libEgl->eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if(this->eglDisplay == EGL_NO_DISPLAY) {
		printf("ERROR: no EGL Display\n");
		throw std::runtime_error("eglGetDisplay");
	}

	int major = 0, minor = 0;
	if(libEgl->eglInitialize(this->eglDisplay, &major, &minor) == EGL_FALSE) {
		printf("ERROR: eglInitialize\n");
		throw std::runtime_error("eglInitialize");
	}
	printf("LibEGL Version: %d.%d\n", major, minor);

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
	if(libEgl->eglChooseConfig(this->eglDisplay, config_attribs, &this->eglConfig, num_configs, &num_configs) == EGL_FALSE
			|| !num_configs || !this->eglConfig) {
		printf("ERROR: eglChooseConfig\n");
		throw std::runtime_error("eglChooseConfig");
	}

	int* properties = 0;
	result = libDisplayInit->dint_create_window(0, this->context, this->width, this->height, properties, &this->dint_window);
	if(result) {
		printf("ERROR: dint_create_window(): %d\n", result);
		throw std::runtime_error("dint_create_window");
	}
	printf("dint_create_window result = %d, native_window = %p, nBuffers = %d\n", result, this->dint_window->native_window, this->dint_window->nBuffers);

	result = libDisplayInit->dint_get_native_window(this->dint_window, (screen_window_t *)&this->native_window);
	if(result || !this->native_window) {
		printf("ERROR: dint_get_native_window(): %d\n", result);
		throw std::runtime_error("dint_get_native_window");
	}
	printf("dint_get_native_window result = %d, native_window = %p\n", result, (void*)this->native_window);

	this->eglSurface = libEgl->eglCreateWindowSurface(this->eglDisplay, this->eglConfig, (EGLNativeWindowType)this->native_window, 0);
	if(this->eglSurface == EGL_NO_SURFACE) {
		printf("ERROR: eglCreateWindowSurface()\n");
		throw std::runtime_error("eglCreateWindowSurface");
	}

	const EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	this->eglContext = libEgl->eglCreateContext(this->eglDisplay, this->eglConfig, EGL_NO_CONTEXT, context_attribs);
	if (eglContext == EGL_NO_CONTEXT) {
		printf("ERROR: eglCreateContext()\n");
		throw std::runtime_error("eglCreateContext");
	}

	if (libEgl->eglMakeCurrent(this->eglDisplay, this->eglSurface, this->eglSurface, this->eglContext) == EGL_FALSE) {
		printf("ERROR: eglMakeCurrent()\n");
		throw std::runtime_error("eglMakeCurrent");
	}
}

void RenderManager::createGlPrograms()
{
    LibGl2* libGl2 = LibGl2::GetInstance();

    // 1. Создание и компиляция вершинного шейдера
    const char* vertexShaderSourceText =
        "attribute vec2 position;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(position, 0.0, 1.0);\n"
        "   gl_PointSize = 1.0;\n"
        "}\n";

    GLuint vertexShaderTextRender = libGl2->glCreateShader(GL_VERTEX_SHADER);
    libGl2->glShaderSource(vertexShaderTextRender, 1, &vertexShaderSourceText, NULL);
    libGl2->glCompileShader(vertexShaderTextRender);

    // Проверка компиляции вершинного шейдера
    GLint vertexShaderCompileStatus;
    libGl2->glGetShaderiv(vertexShaderTextRender, GL_COMPILE_STATUS, &vertexShaderCompileStatus);
    if (vertexShaderCompileStatus != GL_TRUE) {
        char infoLog[512];
        libGl2->glGetShaderInfoLog(vertexShaderTextRender, 512, NULL, infoLog);
        printf("ERROR: Vertex shader compilation failed: %s\n", infoLog);
        libGl2->glDeleteShader(vertexShaderTextRender);
        throw std::runtime_error("Vertex shader compilation failed");
    }

    // 2. Создание и компиляция фрагментного шейдера
    const char* fragmentShaderSourceText =
        "precision mediump float;\n"
        "uniform vec3 textColor;\n"
        "void main()\n"
        "{\n"
        "  gl_FragColor = vec4(textColor, 1.0);\n"
        "}\n";

    GLuint fragmentShaderTextRender = libGl2->glCreateShader(GL_FRAGMENT_SHADER);
    libGl2->glShaderSource(fragmentShaderTextRender, 1, &fragmentShaderSourceText, NULL);
    libGl2->glCompileShader(fragmentShaderTextRender);

    // Проверка компиляции фрагментного шейдера
    GLint fragmentShaderCompileStatus;
    libGl2->glGetShaderiv(fragmentShaderTextRender, GL_COMPILE_STATUS, &fragmentShaderCompileStatus);
    if (fragmentShaderCompileStatus != GL_TRUE) {
        char infoLog[512];
        libGl2->glGetShaderInfoLog(fragmentShaderTextRender, 512, NULL, infoLog);
        printf("ERROR: Fragment shader compilation failed: %s\n", infoLog);
        libGl2->glDeleteShader(vertexShaderTextRender);
        libGl2->glDeleteShader(fragmentShaderTextRender);
        throw std::runtime_error("Fragment shader compilation failed");
    }

    // 3. Создание шейдерной программы
    this->programObjectTextRender = libGl2->glCreateProgram();
    libGl2->glAttachShader(this->programObjectTextRender, vertexShaderTextRender);
    libGl2->glAttachShader(this->programObjectTextRender, fragmentShaderTextRender);
    libGl2->glLinkProgram(this->programObjectTextRender);

    // 4. Проверка линковки программы
    GLint programLinkStatus;
    libGl2->glGetProgramiv(this->programObjectTextRender, GL_LINK_STATUS, &programLinkStatus);
    if (programLinkStatus != GL_TRUE) {
        char infoLog[512];
        libGl2->glGetProgramInfoLog(this->programObjectTextRender, 512, NULL, infoLog);
        printf("ERROR: Program linking failed: %s\n", infoLog);
        libGl2->glDeleteShader(vertexShaderTextRender);
        libGl2->glDeleteShader(fragmentShaderTextRender);
        libGl2->glDeleteProgram(this->programObjectTextRender);
        throw std::runtime_error("Program linking failed");
    }

    // 5. Проверка атрибутов и юниформов
    GLint positionLoc = libGl2->glGetAttribLocation(this->programObjectTextRender, "position");
    if (positionLoc == -1) {
        printf("WARNING: 'position' attribute not found in shader program\n");
    }

    GLint colorLoc = libGl2->glGetUniformLocation(this->programObjectTextRender, "textColor");
    if (colorLoc == -1) {
        printf("WARNING: 'textColor' uniform not found in shader program\n");
    }

    // 6. Удаление шейдеров (они больше не нужны)
    libGl2->glDeleteShader(vertexShaderTextRender);
    libGl2->glDeleteShader(fragmentShaderTextRender);

    printf("glEnable start\n");
    libGl2->glEnable(GL_BLEND);
    printf("glBlendFunc start\n");
    libGl2->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    printf("glDisable start\n");
    libGl2->glDisable(GL_DEPTH_TEST);  // Для 2D-рендеринга

    printf("InitFont start\n");
    if (!InitFont(&font, "/mnt/app/navigation/assets/olympia/olymp1/font/AudiTypeDisplayHigh-Normal.ttf", 24.0f)) {
        printf("ERROR: InitFont()!\n");
    }
    font.windowWidth = this->width;
    font.windowHeight = this->height;
}

void RenderManager::printString(float x, float y, const char* text, float r, float g, float b, float size) {
    if (!text || strlen(text) == 0) return;

    LibGl2* libGl2 = LibGl2::GetInstance();

    char inputBuffer[2000] = { 0 }; // ~500 chars
    GLfloat triangleBuffer[2000] = { 0 };
    int number = stb_easy_font_print(0, 0, text, NULL, inputBuffer, sizeof(inputBuffer));

    // calculate movement inside viewport
    float ndcMovementX = (2.0f * x) / this->width;
    float ndcMovementY = (2.0f * y) / this->height;

    int triangleIndex = 0;
    for (int i = 0; i < sizeof(inputBuffer) / sizeof(GLfloat); i += 8) {
        triangleBuffer[triangleIndex++] = *reinterpret_cast<GLfloat*>(&inputBuffer[i * sizeof(GLfloat)]) / size + ndcMovementX;
        triangleBuffer[triangleIndex++] = *reinterpret_cast<GLfloat*>(&inputBuffer[(i + 1) * sizeof(GLfloat)]) / size * -1 + ndcMovementY;
        triangleBuffer[triangleIndex++] = *reinterpret_cast<GLfloat*>(&inputBuffer[(i + 2) * sizeof(GLfloat)]) / size + +ndcMovementX;
        triangleBuffer[triangleIndex++] = *reinterpret_cast<GLfloat*>(&inputBuffer[(i + 3) * sizeof(GLfloat)]) / size * -1 + ndcMovementY;
        triangleBuffer[triangleIndex++] = *reinterpret_cast<GLfloat*>(&inputBuffer[(i + 4) * sizeof(GLfloat)]) / size + ndcMovementX;
        triangleBuffer[triangleIndex++] = *reinterpret_cast<GLfloat*>(&inputBuffer[(i + 5) * sizeof(GLfloat)]) / size * -1 + ndcMovementY;

        //// Triangle 2
        triangleBuffer[triangleIndex++] = *reinterpret_cast<GLfloat*>(&inputBuffer[i * sizeof(GLfloat)]) / size + ndcMovementX;
        triangleBuffer[triangleIndex++] = *reinterpret_cast<GLfloat*>(&inputBuffer[(i + 1) * sizeof(GLfloat)]) / size * -1 + ndcMovementY;
        triangleBuffer[triangleIndex++] = *reinterpret_cast<GLfloat*>(&inputBuffer[(i + 4) * sizeof(GLfloat)]) / size + ndcMovementX;
        triangleBuffer[triangleIndex++] = *reinterpret_cast<GLfloat*>(&inputBuffer[(i + 5) * sizeof(GLfloat)]) / size * -1 + ndcMovementY;
        triangleBuffer[triangleIndex++] = *reinterpret_cast<GLfloat*>(&inputBuffer[(i + 6) * sizeof(GLfloat)]) / size + ndcMovementX;
        triangleBuffer[triangleIndex++] = *reinterpret_cast<GLfloat*>(&inputBuffer[(i + 7) * sizeof(GLfloat)]) / size * -1 + ndcMovementY;
    }

    libGl2->glUseProgram(this->programObjectTextRender);

    GLint colorLoc = libGl2->glGetUniformLocation(this->programObjectTextRender, "textColor");
    if (colorLoc != -1) libGl2->glUniform3f(colorLoc, r, g, b);

    GLuint vbo;
    libGl2->glGenBuffers(1, &vbo);
    libGl2->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    libGl2->glBufferData(GL_ARRAY_BUFFER, triangleIndex * sizeof(GLfloat), triangleBuffer, GL_STATIC_DRAW);

    GLint positionLoc = libGl2->glGetAttribLocation(this->programObjectTextRender, "position");
    if (positionLoc == -1) {
        printf("ERROR: 'position' attribute not found\n");
        libGl2->glDeleteBuffers(1, &vbo);
        return;
    }

    libGl2->glEnableVertexAttribArray(positionLoc);
    libGl2->glVertexAttribPointer(positionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

    libGl2->glDrawArrays(GL_TRIANGLES, 0, triangleIndex / 2); // по 2 float на вершину

    libGl2->glDisableVertexAttribArray(positionLoc);
    libGl2->glDeleteBuffers(1, &vbo);
}

void RenderManager::addRender(IRender* render)
{
	if(!render) {
		printf("Warning: render is null\n");
		return;
	}
	render->setManager(this);
	this->renders.push_back(render);
}

void RenderManager::render()
{
	LibGl2* libGl2 = LibGl2::GetInstance();
	LibEgl* libEgl = LibEgl::GetInstance();

	libGl2->glViewport(0, 0, this->width, this->height);
	libGl2->glClear(GL_COLOR_BUFFER_BIT);

	for(size_t i = 0; i < this->renders.size(); ++i)
	{
		if(!renders[i]) {
			printf("Warning: renders[%d] is null\n", i);
			continue;
		}
		try {
			renders[i]->render();
		}
		catch(...) {
			printf("Error occurred while call renders[%d]->render()\n", i);
		}
	}

	libEgl->eglSwapBuffers(this->eglDisplay, this->eglSurface);
}

void RenderManager::render(const std::wstring& renderStr)
{
	printf("Render with wstring start\n");
	char* utf8 = wstring_to_utf8_cstr(renderStr);
	printf("wstring_to_utf8_cstr ok \n");

	this->render(utf8);

	if(utf8) {
		free(utf8);
	}
	printf("Render with wstring done\n");
}

void RenderManager::render(const char* renderStr)
{
	printf("Render with char* start\n");
	LibGl2* libGl2 = LibGl2::GetInstance();
	LibEgl* libEgl = LibEgl::GetInstance();

	libGl2->glViewport(0, 0, this->width, this->height);
	//libGl2->glClearColor(0.0, 1.0, 0.0, 1.0); // Зелёный фон
	libGl2->glClear(GL_COLOR_BUFFER_BIT);
	//printString(-300, 100, "Q3 F3 Team OpenGL", 1.0, 0.0, 0.0, 200.0);
	RenderText(&font, renderStr, 50, 150, 1.0f, 1.0f, 1.0f);
	libEgl->eglSwapBuffers(this->eglDisplay, this->eglSurface);
	printf("Render with char* ok\n");
}
