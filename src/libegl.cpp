#include "libegl.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

LibEgl* LibEgl::_singleton= nullptr;

#define LOAD_EGL_FUNC(func) \
    this->func = (func##_handle)dlsym(this->_func_handle, #func); \
    if (!this->func) { \
        printf("Error while loading %s function %s: %s\n", EGL_LIB_NAME, #func, dlerror()); \
        exit(EXIT_FAILURE); \
    }

LibEgl::LibEgl()
{
	printf("LibEgl start\n");

	this->_func_handle = 0;

	this->_func_handle = dlopen(EGL_LIB_NAME, RTLD_NOW);
	if (!this->_func_handle) {
		printf("Error using %s: %s\n", EGL_LIB_NAME, dlerror());
		exit(EXIT_FAILURE);
	}

	// Загрузка всех EGL-функций
	LOAD_EGL_FUNC(eglGetError)
	LOAD_EGL_FUNC(eglGetDisplay)
	LOAD_EGL_FUNC(eglInitialize)
	LOAD_EGL_FUNC(eglTerminate)
	LOAD_EGL_FUNC(eglQueryString)
	LOAD_EGL_FUNC(eglGetConfigs)
	LOAD_EGL_FUNC(eglChooseConfig)
	LOAD_EGL_FUNC(eglGetConfigAttrib)
	LOAD_EGL_FUNC(eglCreateWindowSurface)
	LOAD_EGL_FUNC(eglCreatePbufferSurface)
	LOAD_EGL_FUNC(eglCreatePixmapSurface)
	LOAD_EGL_FUNC(eglDestroySurface)
	LOAD_EGL_FUNC(eglQuerySurface)
	LOAD_EGL_FUNC(eglBindAPI)
	LOAD_EGL_FUNC(eglQueryAPI)
	LOAD_EGL_FUNC(eglWaitClient)
	LOAD_EGL_FUNC(eglReleaseThread)
	LOAD_EGL_FUNC(eglCreatePbufferFromClientBuffer)
	LOAD_EGL_FUNC(eglBindTexImage)
	LOAD_EGL_FUNC(eglReleaseTexImage)
	LOAD_EGL_FUNC(eglSwapInterval)
	LOAD_EGL_FUNC(eglCreateContext)
	LOAD_EGL_FUNC(eglDestroyContext)
	LOAD_EGL_FUNC(eglMakeCurrent)
	LOAD_EGL_FUNC(eglGetCurrentContext)
	LOAD_EGL_FUNC(eglGetCurrentSurface)
	LOAD_EGL_FUNC(eglGetCurrentDisplay)
	LOAD_EGL_FUNC(eglQueryContext)
	LOAD_EGL_FUNC(eglWaitGL)
	LOAD_EGL_FUNC(eglWaitNative)
	LOAD_EGL_FUNC(eglSwapBuffers)
	LOAD_EGL_FUNC(eglCopyBuffers)
	LOAD_EGL_FUNC(eglSurfaceAttrib)

	printf("LibEgl successful\n");
}

LibEgl::~LibEgl()
{
	printf("~LibEgl start\n");

	if(this->_func_handle)
	{
		if(dlclose(this->_func_handle) != 0)
		{
			printf("Error while closing %s handle: %s\n", EGL_LIB_NAME, dlerror());
		}
	}

	printf("~LibEgl successful\n");
}

LibEgl* LibEgl::GetInstance()
{
    if(_singleton == nullptr){
    	_singleton = new LibEgl();
    }
    return _singleton;
}
