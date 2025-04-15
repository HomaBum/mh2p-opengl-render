#include "RenderManager.h"
#include <stdexcept>
#include "libegl.h"
#include "libgl2.h"
#include "stb_easyfont.hh"

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
}

void RenderManager::printString(float x, float y, const char* text, float r, float g, float b, float size) {
    LibGl2* libGl2 = LibGl2::GetInstance();

    // 1. Проверка входных данных
    if (!text || strlen(text) == 0) {
        printf("WARNING: Empty text string\n");
        return;
    }

    // 2. Генерация геометрии текста
    const int MAX_BUFFER_SIZE = 2000;
    char inputBuffer[MAX_BUFFER_SIZE] = { 0 };
    GLfloat triangleBuffer[MAX_BUFFER_SIZE] = { 0 };

    int numChars = stb_easy_font_print(0, 0, text, NULL, inputBuffer, sizeof(inputBuffer));
    if (numChars <= 0) {
        printf("WARNING: No characters generated\n");
        return;
    }

    // 3. Преобразование координат в NDC
    float scaleX = (2.0f * size / 10.0f) / width;   // Масштабирование с учетом размера
    float scaleY = (2.0f * size / 10.0f) / height;
    float offsetX = (2.0f * x / 100.0f) / width;    // Нормализация смещения
    float offsetY = (2.0f * y / 100.0f) / height;

    // 4. Преобразование квадов в треугольники
    int triangleIndex = 0;
    const int MAX_VERTICES = MAX_BUFFER_SIZE / sizeof(GLfloat);

    for (int i = 0; i < numChars * 8 && triangleIndex < MAX_VERTICES - 12; i += 8) {
        // Первый треугольник
        triangleBuffer[triangleIndex++] = inputBuffer[i] * scaleX + offsetX;
        triangleBuffer[triangleIndex++] = -(inputBuffer[i+1] * scaleY + offsetY);

        triangleBuffer[triangleIndex++] = inputBuffer[i+2] * scaleX + offsetX;
        triangleBuffer[triangleIndex++] = -(inputBuffer[i+3] * scaleY + offsetY);

        triangleBuffer[triangleIndex++] = inputBuffer[i+4] * scaleX + offsetX;
        triangleBuffer[triangleIndex++] = -(inputBuffer[i+5] * scaleY + offsetY);

        // Второй треугольник
        triangleBuffer[triangleIndex++] = inputBuffer[i] * scaleX + offsetX;
        triangleBuffer[triangleIndex++] = -(inputBuffer[i+1] * scaleY + offsetY);

        triangleBuffer[triangleIndex++] = inputBuffer[i+4] * scaleX + offsetX;
        triangleBuffer[triangleIndex++] = -(inputBuffer[i+5] * scaleY + offsetY);

        triangleBuffer[triangleIndex++] = inputBuffer[i+6] * scaleX + offsetX;
        triangleBuffer[triangleIndex++] = -(inputBuffer[i+7] * scaleY + offsetY);
    }

    // 5. Проверка, что есть что рисовать
    if (triangleIndex == 0) {
        printf("WARNING: No vertices generated\n");
        return;
    }

    // 6. Настройка шейдера
    libGl2->glUseProgram(this->programObjectTextRender);

    // Установка цвета текста
    GLint colorLoc = libGl2->glGetUniformLocation(this->programObjectTextRender, "textColor");
    if (colorLoc != -1) {
        libGl2->glUniform3f(colorLoc, r, g, b);
    } else {
        printf("WARNING: 'textColor' uniform not found\n");
    }

    // 7. Создание и настройка VBO
    GLuint vbo;
    libGl2->glGenBuffers(1, &vbo);
    libGl2->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    libGl2->glBufferData(GL_ARRAY_BUFFER, triangleIndex * sizeof(GLfloat), triangleBuffer, GL_STATIC_DRAW);

    // 8. Настройка атрибута position
    GLint positionLoc = libGl2->glGetAttribLocation(this->programObjectTextRender, "position");
    if (positionLoc == -1) {
        printf("ERROR: 'position' attribute not found\n");
        libGl2->glDeleteBuffers(1, &vbo);
        return;
    }

    libGl2->glEnableVertexAttribArray(positionLoc);
    libGl2->glVertexAttribPointer(positionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // 9. Отрисовка
    libGl2->glDrawArrays(GL_TRIANGLES, 0, triangleIndex / 2); // 2 компоненты на вершину

    // 10. Очистка
    libGl2->glDisableVertexAttribArray(positionLoc);
    libGl2->glDeleteBuffers(1, &vbo);

    // Проверка ошибок OpenGL
    GLenum err = libGl2->glGetError();
    if (err != GL_NO_ERROR) {
        printf("OpenGL error during text rendering: 0x%x\n", err);
    }
}

void RenderManager::render()
{
	LibGl2* libGl2 = LibGl2::GetInstance();
	LibEgl* libEgl = LibEgl::GetInstance();

	libGl2->glClearColor(0.0, 1.0, 0.0, 1.0); // Зелёный фон
	libGl2->glClear(GL_COLOR_BUFFER_BIT);
	this->printString(0, 0, "Q3 F3 Team OpenGL", 1.0, 0.0, 0.0, 40.0); // Красный текст в центре
	libEgl->eglSwapBuffers(this->eglDisplay, this->eglSurface);
}
