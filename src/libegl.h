#ifndef LIBEGL_H
#define LIBEGL_H

#include <EGL/egl.h>

#define EGL_LIB_NAME "libEGL.so.1"

class LibEgl {

	public:

	private:
		void* _func_handle;

		typedef EGLint (*eglGetError_handle)(void);
		typedef EGLDisplay (*eglGetDisplay_handle)(EGLNativeDisplayType display_id);
		typedef EGLBoolean (*eglInitialize_handle)(EGLDisplay dpy, EGLint *major, EGLint *minor);
		typedef EGLBoolean (*eglTerminate_handle)(EGLDisplay dpy);
		typedef const char* (*eglQueryString_handle)(EGLDisplay dpy, EGLint name);
		typedef EGLBoolean (*eglGetConfigs_handle)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
		typedef EGLBoolean (*eglChooseConfig_handle)(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
		typedef EGLBoolean (*eglGetConfigAttrib_handle)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
		typedef EGLSurface (*eglCreateWindowSurface_handle)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
		typedef EGLSurface (*eglCreatePbufferSurface_handle)(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
		typedef EGLSurface (*eglCreatePixmapSurface_handle)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list);
		typedef EGLBoolean (*eglDestroySurface_handle)(EGLDisplay dpy, EGLSurface  surface);
		typedef EGLBoolean (*eglQuerySurface_handle)(EGLDisplay dpy, EGLSurface  surface, EGLint attribute, EGLint *value);
		typedef EGLBoolean (*eglBindAPI_handle)(EGLenum api);
		typedef EGLenum (*eglQueryAPI_handle)(void);
		typedef EGLBoolean (*eglWaitClient_handle)(void);
		typedef EGLBoolean (*eglReleaseThread_handle)(void);
		typedef EGLSurface (*eglCreatePbufferFromClientBuffer_handle)(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list);
		typedef EGLBoolean (*eglSurfaceAttrib_handle)(EGLDisplay dpy, EGLSurface  surface, EGLint attribute, EGLint value);
		typedef EGLBoolean (*eglBindTexImage_handle)(EGLDisplay dpy, EGLSurface  surface, EGLint buffer);
		typedef EGLBoolean (*eglReleaseTexImage_handle)(EGLDisplay dpy, EGLSurface  surface, EGLint buffer);
		typedef EGLBoolean (*eglSwapInterval_handle)(EGLDisplay dpy, EGLint interval);
		typedef EGLContext (*eglCreateContext_handle)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
		typedef EGLBoolean (*eglDestroyContext_handle)(EGLDisplay dpy, EGLContext ctx);
		typedef EGLBoolean (*eglMakeCurrent_handle)(EGLDisplay dpy, EGLSurface  draw, EGLSurface  read, EGLContext ctx);
		typedef EGLContext (*eglGetCurrentContext_handle)(void);
		typedef EGLSurface (*eglGetCurrentSurface_handle)(EGLint readdraw);
		typedef EGLDisplay (*eglGetCurrentDisplay_handle)(void);
		typedef EGLBoolean (*eglQueryContext_handle)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
		typedef EGLBoolean (*eglWaitGL_handle)(void);
		typedef EGLBoolean (*eglWaitNative_handle)(EGLint engine);
		typedef EGLBoolean (*eglSwapBuffers_handle)(EGLDisplay dpy, EGLSurface  surface);
		typedef EGLBoolean (*eglCopyBuffers_handle)(EGLDisplay dpy, EGLSurface  surface, EGLNativePixmapType target);

	protected:
		static LibEgl* _singleton;

		LibEgl();
		~LibEgl();

	public:
		LibEgl(LibEgl &other) = delete;
		void operator=(const LibEgl &) = delete;
		static LibEgl *GetInstance();

		eglGetError_handle eglGetError;
		eglGetDisplay_handle eglGetDisplay;
		eglInitialize_handle eglInitialize;
		eglTerminate_handle eglTerminate;
		eglQueryString_handle eglQueryString;
		eglGetConfigs_handle eglGetConfigs;
		eglChooseConfig_handle eglChooseConfig;
		eglGetConfigAttrib_handle eglGetConfigAttrib;
		eglCreateWindowSurface_handle eglCreateWindowSurface;
		eglCreatePbufferSurface_handle eglCreatePbufferSurface;
		eglCreatePixmapSurface_handle eglCreatePixmapSurface;
		eglDestroySurface_handle eglDestroySurface;
		eglQuerySurface_handle eglQuerySurface;
		eglBindAPI_handle eglBindAPI;
		eglQueryAPI_handle eglQueryAPI;
		eglWaitClient_handle eglWaitClient;
		eglReleaseThread_handle eglReleaseThread;
		eglCreatePbufferFromClientBuffer_handle eglCreatePbufferFromClientBuffer;
		eglSurfaceAttrib_handle eglSurfaceAttrib;
		eglBindTexImage_handle eglBindTexImage;
		eglReleaseTexImage_handle eglReleaseTexImage;
		eglSwapInterval_handle eglSwapInterval;
		eglCreateContext_handle eglCreateContext;
		eglDestroyContext_handle eglDestroyContext;
		eglMakeCurrent_handle eglMakeCurrent;
		eglGetCurrentContext_handle eglGetCurrentContext;
		eglGetCurrentSurface_handle eglGetCurrentSurface;
		eglGetCurrentDisplay_handle eglGetCurrentDisplay;
		eglQueryContext_handle eglQueryContext;
		eglWaitGL_handle eglWaitGL;
		eglWaitNative_handle eglWaitNative;
		eglSwapBuffers_handle eglSwapBuffers;
		eglCopyBuffers_handle eglCopyBuffers;

};

#endif
