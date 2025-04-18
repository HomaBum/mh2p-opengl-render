#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H

#include "libdisplayinit.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "DisplayableConsts.h"
#include "text_render.h"
#include <string>
#include <vector>

class IRender
{
    public:
		virtual ~IRender() {}
        virtual void render() = 0;
        virtual void setManager(class RenderManager* manager) = 0;
};

class RenderManager {

	public:

	private:
		int context;
		int width;
		int height;

		LibDisplayInit::dint_screen_window* dint_window;
		int native_window;

		EGLDisplay eglDisplay;
		EGLSurface eglSurface;
		EGLContext eglContext;
		EGLConfig eglConfig;

		GLuint programObjectTextRender;

		FontData font;

		std::vector<IRender*> renders;

		void init();
		void createGlPrograms();
		void printString(float x, float y, const char* text, float r, float g, float b, float size);
	public:
		RenderManager(int ctx, int w, int h);
		~RenderManager();

		void addRender(IRender* render);

		void render();

		void render(const std::wstring& renderStr);
		void render(const char* renderStr);
};

#endif
