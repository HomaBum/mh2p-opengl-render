#include "libgl2.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

LibGl2* LibGl2::_singleton= nullptr;

#define LOAD_GL_FUNC(func) \
    this->func = (func##_handle)dlsym(this->_func_handle, #func); \
    if (!this->func) { \
        printf("Error while loading %s function %s: %s\n", GL2_LIB_NAME, #func, dlerror()); \
        exit(EXIT_FAILURE); \
    }

LibGl2::LibGl2()
{
	printf("LibGl2 start\n");

	this->_func_handle = 0;

	this->_func_handle = dlopen(GL2_LIB_NAME, RTLD_NOW);
	if (!this->_func_handle) {
		printf("Error using %s: %s\n", GL2_LIB_NAME, dlerror());
		exit(EXIT_FAILURE);
	}

	// Load all GL functions
	LOAD_GL_FUNC(glActiveTexture)
	LOAD_GL_FUNC(glAttachShader)
	LOAD_GL_FUNC(glBindAttribLocation)
	LOAD_GL_FUNC(glBindBuffer)
	LOAD_GL_FUNC(glBindFramebuffer)
	LOAD_GL_FUNC(glBindRenderbuffer)
	LOAD_GL_FUNC(glBindTexture)
	LOAD_GL_FUNC(glBlendColor)
	LOAD_GL_FUNC(glBlendEquation)
	LOAD_GL_FUNC(glBlendEquationSeparate)
	LOAD_GL_FUNC(glBlendFunc)
	LOAD_GL_FUNC(glBlendFuncSeparate)
	LOAD_GL_FUNC(glBufferData)
	LOAD_GL_FUNC(glBufferSubData)
	LOAD_GL_FUNC(glCheckFramebufferStatus)
	LOAD_GL_FUNC(glClear)
	LOAD_GL_FUNC(glClearColor)
	LOAD_GL_FUNC(glClearDepthf)
	LOAD_GL_FUNC(glClearStencil)
	LOAD_GL_FUNC(glColorMask)
	LOAD_GL_FUNC(glCompileShader)
	LOAD_GL_FUNC(glCompressedTexImage2D)
	LOAD_GL_FUNC(glCompressedTexSubImage2D)
	LOAD_GL_FUNC(glCopyTexImage2D)
	LOAD_GL_FUNC(glCopyTexSubImage2D)
	LOAD_GL_FUNC(glCreateProgram)
	LOAD_GL_FUNC(glCreateShader)
	LOAD_GL_FUNC(glCullFace)
	LOAD_GL_FUNC(glDeleteBuffers)
	LOAD_GL_FUNC(glDeleteFramebuffers)
	LOAD_GL_FUNC(glDeleteProgram)
	LOAD_GL_FUNC(glDeleteRenderbuffers)
	LOAD_GL_FUNC(glDeleteShader)
	LOAD_GL_FUNC(glDeleteTextures)
	LOAD_GL_FUNC(glDepthFunc)
	LOAD_GL_FUNC(glDepthMask)
	LOAD_GL_FUNC(glDepthRangef)
	LOAD_GL_FUNC(glDetachShader)
	LOAD_GL_FUNC(glDisable)
	LOAD_GL_FUNC(glDisableVertexAttribArray)
	LOAD_GL_FUNC(glDrawArrays)
	LOAD_GL_FUNC(glDrawElements)
	LOAD_GL_FUNC(glEnable)
	LOAD_GL_FUNC(glEnableVertexAttribArray)
	LOAD_GL_FUNC(glFinish)
	LOAD_GL_FUNC(glFlush)
	LOAD_GL_FUNC(glFramebufferRenderbuffer)
	LOAD_GL_FUNC(glFramebufferTexture2D)
	LOAD_GL_FUNC(glFrontFace)
	LOAD_GL_FUNC(glGenBuffers)
	LOAD_GL_FUNC(glGenerateMipmap)
	LOAD_GL_FUNC(glGenFramebuffers)
	LOAD_GL_FUNC(glGenRenderbuffers)
	LOAD_GL_FUNC(glGenTextures)
	LOAD_GL_FUNC(glGetActiveAttrib)
	LOAD_GL_FUNC(glGetActiveUniform)
	LOAD_GL_FUNC(glGetAttachedShaders)
	LOAD_GL_FUNC(glGetAttribLocation)
	LOAD_GL_FUNC(glGetBooleanv)
	LOAD_GL_FUNC(glGetBufferParameteriv)
	LOAD_GL_FUNC(glGetError)
	LOAD_GL_FUNC(glGetFloatv)
	LOAD_GL_FUNC(glGetFramebufferAttachmentParameteriv)
	LOAD_GL_FUNC(glGetIntegerv)
	LOAD_GL_FUNC(glGetProgramiv)
	LOAD_GL_FUNC(glGetProgramInfoLog)
	LOAD_GL_FUNC(glGetRenderbufferParameteriv)
	LOAD_GL_FUNC(glGetShaderiv)
	LOAD_GL_FUNC(glGetShaderInfoLog)
	LOAD_GL_FUNC(glGetShaderPrecisionFormat)
	LOAD_GL_FUNC(glGetShaderSource)
	LOAD_GL_FUNC(glGetString)
	LOAD_GL_FUNC(glGetTexParameterfv)
	LOAD_GL_FUNC(glGetTexParameteriv)
	LOAD_GL_FUNC(glGetUniformfv)
	LOAD_GL_FUNC(glGetUniformiv)
	LOAD_GL_FUNC(glGetUniformLocation)
	LOAD_GL_FUNC(glGetVertexAttribfv)
	LOAD_GL_FUNC(glGetVertexAttribiv)
	LOAD_GL_FUNC(glGetVertexAttribPointerv)
	LOAD_GL_FUNC(glHint)
	LOAD_GL_FUNC(glIsBuffer)
	LOAD_GL_FUNC(glIsEnabled)
	LOAD_GL_FUNC(glIsFramebuffer)
	LOAD_GL_FUNC(glIsProgram)
	LOAD_GL_FUNC(glIsRenderbuffer)
	LOAD_GL_FUNC(glIsShader)
	LOAD_GL_FUNC(glIsTexture)
	LOAD_GL_FUNC(glLineWidth)
	LOAD_GL_FUNC(glLinkProgram)
	LOAD_GL_FUNC(glPixelStorei)
	LOAD_GL_FUNC(glPolygonOffset)
	LOAD_GL_FUNC(glReadPixels)
	LOAD_GL_FUNC(glReleaseShaderCompiler)
	LOAD_GL_FUNC(glRenderbufferStorage)
	LOAD_GL_FUNC(glSampleCoverage)
	LOAD_GL_FUNC(glScissor)
	LOAD_GL_FUNC(glShaderBinary)
	LOAD_GL_FUNC(glShaderSource)
	LOAD_GL_FUNC(glStencilFunc)
	LOAD_GL_FUNC(glStencilFuncSeparate)
	LOAD_GL_FUNC(glStencilMask)
	LOAD_GL_FUNC(glStencilMaskSeparate)
	LOAD_GL_FUNC(glStencilOp)
	LOAD_GL_FUNC(glStencilOpSeparate)
	LOAD_GL_FUNC(glTexImage2D)
	LOAD_GL_FUNC(glTexParameterf)
	LOAD_GL_FUNC(glTexParameterfv)
	LOAD_GL_FUNC(glTexParameteri)
	LOAD_GL_FUNC(glTexParameteriv)
	LOAD_GL_FUNC(glTexSubImage2D)
	LOAD_GL_FUNC(glUniform1f)
	LOAD_GL_FUNC(glUniform1fv)
	LOAD_GL_FUNC(glUniform1i)
	LOAD_GL_FUNC(glUniform1iv)
	LOAD_GL_FUNC(glUniform2f)
	LOAD_GL_FUNC(glUniform2fv)
	LOAD_GL_FUNC(glUniform2i)
	LOAD_GL_FUNC(glUniform2iv)
	LOAD_GL_FUNC(glUniform3f)
	LOAD_GL_FUNC(glUniform3fv)
	LOAD_GL_FUNC(glUniform3i)
	LOAD_GL_FUNC(glUniform3iv)
	LOAD_GL_FUNC(glUniform4f)
	LOAD_GL_FUNC(glUniform4fv)
	LOAD_GL_FUNC(glUniform4i)
	LOAD_GL_FUNC(glUniform4iv)
	LOAD_GL_FUNC(glUniformMatrix2fv)
	LOAD_GL_FUNC(glUniformMatrix3fv)
	LOAD_GL_FUNC(glUniformMatrix4fv)
	LOAD_GL_FUNC(glUseProgram)
	LOAD_GL_FUNC(glValidateProgram)
	LOAD_GL_FUNC(glVertexAttrib1f)
	LOAD_GL_FUNC(glVertexAttrib1fv)
	LOAD_GL_FUNC(glVertexAttrib2f)
	LOAD_GL_FUNC(glVertexAttrib2fv)
	LOAD_GL_FUNC(glVertexAttrib3f)
	LOAD_GL_FUNC(glVertexAttrib3fv)
	LOAD_GL_FUNC(glVertexAttrib4f)
	LOAD_GL_FUNC(glVertexAttrib4fv)
	LOAD_GL_FUNC(glVertexAttribPointer)
	LOAD_GL_FUNC(glViewport)

	printf("LibGl2 successful\n");
}

LibGl2::~LibGl2()
{
	printf("~LibGl2 start\n");

	if(this->_func_handle)
	{
		if(dlclose(this->_func_handle) != 0)
		{
			printf("Error while closing %s handle: %s\n", GL2_LIB_NAME, dlerror());
		}
	}

	printf("~LibGl2 successful\n");
}

LibGl2* LibGl2::GetInstance()
{
    if(_singleton == nullptr){
    	_singleton = new LibGl2();
    }
    return _singleton;
}
