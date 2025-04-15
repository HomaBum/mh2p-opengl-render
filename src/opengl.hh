#ifndef INCLUDE_OPENGL_H
#define INCLUDE_OPENGL_H

#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/keycodes.h>
#include <time.h>
#include "stb_easyfont.hh"
#include <regex.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <dlfcn.h>   // for dynamic loading functions such as dlopen, 	lsym, and dlclose
#include <iostream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "miniz.h"
#include <unistd.h>
#include <sys/time.h>

#include <netinet/tcp.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/sysctl.h>
#include <sys/param.h>
#include <netinet/tcp_var.h>

#include "libegl.h"
#include "libgl2.h"

LibEgl* libEgl = LibEgl::GetInstance();
LibGl2* libGl2 = LibGl2::GetInstance();

#ifndef TCP_USER_TIMEOUT
#define TCP_USER_TIMEOUT 18  // how long for loss retry before timeout [ms]
#endif

//GLES setup
GLuint programObject;
GLuint programObjectTextRender;
EGLDisplay eglDisplay;
EGLSurface eglSurface;
EGLContext eglContext;

// VNC shaders
// Vertex shader source
const char* vertexShaderSource = "attribute vec2 position;    \n"
	"attribute vec2 texCoord;     \n" // Add texture coordinate attribute
		"varying vec2 v_texCoord;     \n" // Declare varying variable for texture coordinate
		"void main()                  \n"
		"{                            \n"
		"   gl_Position = vec4(position, 0.0, 1.0); \n"
		"   v_texCoord = texCoord;   \n"
		"   gl_PointSize = 4.0;      \n" // Point size
		"}                            \n";

const char* fragmentShaderSource = "precision mediump float;\n"
	"varying vec2 v_texCoord;\n"
	"uniform sampler2D texture;\n"
	"void main()\n"
	"{\n"
	"    gl_FragColor = texture2D(texture, v_texCoord);\n"
	"}\n";

// Text Rendering shaders
const char* vertexShaderSourceText = "attribute vec2 position;    \n"
	"void main()                  \n"
	"{                            \n"
	"   gl_Position = vec4(position, 0.0, 1.0); \n"
	"   gl_PointSize = 1.0;      \n" // Point size
		"}                            \n";

// Fragment shader source
const char* fragmentShaderSourceText = "precision mediump float;\n"
	"void main()               \n"
	"{                         \n"
	"  gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); \n" // Color
		"}                         \n";

GLfloat landscapeVertices[] = { -0.8f, 0.73, 0.0f, // Top Left
		0.8f, 0.73f, 0.0f, // Top Right
		0.8f, -0.63f, 0.0f, // Bottom Right
		-0.8f, -0.63f, 0.0f // Bottom Left
		};
GLfloat portraitVertices[] = { -0.8f, 1.0f, 0.0f, // Top Left
		0.8f, 1.0f, 0.0f, // Top Right
		0.8f, -0.67f, 0.0f, // Bottom Right
		-0.8f, -0.67f, 0.0f // Bottom Left
		};
// Texture coordinates
GLfloat landscapeTexCoords[] = { 0.0f, 0.07f, // Bottom Left
		0.90f, 0.07f, // Bottom Right
		0.90f, 1.0f, // Top Right
		0.0f, 1.0f // Top Left
		};
// Texture coordinates
GLfloat portraitTexCoords[] = { 0.0f, 0.0f, // Bottom Left
		0.63f, 0.0f, // Bottom Right
		0.63f, 0.3f, // Top Right
		0.0f, 0.3f // Top Left
		};

// Constants for VNC protocol
const char* PROTOCOL_VERSION = "RFB 003.003\n"; // Client initialization message
const char FRAMEBUFFER_UPDATE_REQUEST[] = { 3, 0, 0, 0, 0, 0, 255, 255, 255,
		255 };
const char CLIENT_INIT[] = { 1 };
const char ZLIB_ENCODING[] = { 2, 0, 0, 2, 0, 0, 0, 6, 0, 0, 0, 0 };

// SETUP SECTION
int windowWidth = 1920;
int windowHeight = 722;

const char* VNC_SERVER_IP_ADDRESS = "10.173.189.62";
const int VNC_SERVER_PORT = 5900;

const char* EXLAP_SERVER_IP_ADDRESS = "127.0.0.1";
const int EXLAP_SERVER_PORT = 25010;

// QNX SPECIFIC SECTION
static EGLenum checkErrorEGL(const char* msg) {
	static const char
			* errmsg[] = {
					"EGL function succeeded",
					"EGL is not initialized, or could not be initialized, for the specified display",
					"EGL cannot access a requested resource",
					"EGL failed to allocate resources for the requested operation",
					"EGL fail to access an unrecognized attribute or attribute value was passed in an attribute list",
					"EGLConfig argument does not name a valid EGLConfig",
					"EGLContext argument does not name a valid EGLContext",
					"EGL current surface of the calling thread is no longer valid",
					"EGLDisplay argument does not name a valid EGLDisplay",
					"EGL arguments are inconsistent",
					"EGLNativePixmapType argument does not refer to a valid native pixmap",
					"EGLNativeWindowType argument does not refer to a valid native window",
					"EGL one or more argument values are invalid",
					"EGLSurface argument does not name a valid surface configured for rendering",
					"EGL power management event has occurred", };
	EGLenum error = libEgl->eglGetError();
	fprintf(stderr, "%s: %s\n", msg, errmsg[error - EGL_SUCCESS]);
	return error;
}

struct Command {
	const char* command;
	const char* error_message;
};

// CODE FROM HERE IS THE SAME FOR WINDOWS OR QNX
void execute_initial_commands() {
	printf("execute_initial_commands() start\n");
	struct Command commands[] = { { "/eso/bin/apps/dmdt dc 70 3",
			"Create new display table with context 3 failed with error" }, {
			"/eso/bin/apps/dmdt sc 4 70",
			"Set display 4 (VC) to display table 99 failed with error" } };
	size_t num_commands = sizeof(commands) / sizeof(commands[0]);

	for (size_t i = 0; i < num_commands; ++i) {
		const char* command = commands[i].command;
		const char* error_message = commands[i].error_message;
		printf("Executing '%s'\n", command);

		// Execute the command
		int ret = system(command);
		if (ret != 0) {
			fprintf(stderr, "%s: %d\n", error_message, ret);
		}
	}
	printf("execute_initial_commands() end\n");
}

void execute_final_commands() {
	printf("execute_final_commands() start\n");
	struct Command commands[] = { { "/eso/bin/apps/dmdt dc 70 33",
			"Create new display table with context 3 failed with error" }, {
			"/eso/bin/apps/dmdt sc 4 70",
			"Set display 4 (VC) to display table 99 failed with error" } };
	size_t num_commands = sizeof(commands) / sizeof(commands[0]);

	for (size_t i = 0; i < num_commands; ++i) {
		const char* command = commands[i].command;
		const char* error_message = commands[i].error_message;
		printf("Executing '%s'\n", command);

		// Execute the command
		int ret = system(command);
		if (ret != 0) {
			fprintf(stderr, "%s: %d\n", error_message, ret);
		}
	}
	printf("execute_final_commands() end\n");
}

int16_t byteArrayToInt16(const char* byteArray) {
	return ((int16_t) (byteArray[0] & 0xFF) << 8) | (byteArray[1] & 0xFF);
}

int32_t byteArrayToInt32(const char* byteArray) {
	return ((int32_t) (byteArray[0] & 0xFF) << 24) | ((int32_t) (byteArray[1]
			& 0xFF) << 16) | ((int32_t) (byteArray[2] & 0xFF) << 8)
			| (byteArray[3] & 0xFF);
}

// Initialize OpenGL ES
void Init() {
	// Load and compile VNC shaders
	GLuint vertexShaderVncRender = libGl2->glCreateShader(GL_VERTEX_SHADER);
	libGl2->glShaderSource(vertexShaderVncRender, 1, &vertexShaderSource, NULL);
	libGl2->glCompileShader(vertexShaderVncRender);

	// Check for compile errors
	GLint vertexShaderCompileStatus;
	libGl2->glGetShaderiv(vertexShaderVncRender, GL_COMPILE_STATUS,
			&vertexShaderCompileStatus);
	if (vertexShaderCompileStatus != GL_TRUE) {
		char infoLog[512];
		libGl2->glGetShaderInfoLog(vertexShaderVncRender, 512, NULL, infoLog);
		printf("Vertex shader compilation failed: %s\n", infoLog);
	}

	GLuint fragmentShaderVncRender = libGl2->glCreateShader(GL_FRAGMENT_SHADER);
	libGl2->glShaderSource(fragmentShaderVncRender, 1, &fragmentShaderSource, NULL);
	libGl2->glCompileShader(fragmentShaderVncRender);

	// Check for compile errors
	GLint fragmentShaderCompileStatus;
	libGl2->glGetShaderiv(fragmentShaderVncRender, GL_COMPILE_STATUS,
			&fragmentShaderCompileStatus);
	if (fragmentShaderCompileStatus != GL_TRUE) {
		char infoLog[512];
		libGl2->glGetShaderInfoLog(fragmentShaderVncRender, 512, NULL, infoLog);
		printf("Fragment shader compilation failed: %s\n", infoLog);
	}

	// Load and compile Text Render shaders
	GLuint vertexShaderTextRender = libGl2->glCreateShader(GL_VERTEX_SHADER);
	libGl2->glShaderSource(vertexShaderTextRender, 1, &vertexShaderSourceText, NULL);
	libGl2->glCompileShader(vertexShaderTextRender);

	// Check for compile errors
	GLint vertexShaderTextRenderCompileStatus;
	libGl2->glGetShaderiv(vertexShaderTextRender, GL_COMPILE_STATUS,
			&vertexShaderTextRenderCompileStatus);
	if (vertexShaderTextRenderCompileStatus != GL_TRUE) {
		char infoLog[512];
		libGl2->glGetShaderInfoLog(vertexShaderTextRender, 512, NULL, infoLog);
		printf("Vertex shader compilation failed: %s\n", infoLog);
	}

	GLuint fragmentShaderTextRender = libGl2->glCreateShader(GL_FRAGMENT_SHADER);
	libGl2->glShaderSource(fragmentShaderTextRender, 1, &fragmentShaderSourceText, NULL);
	libGl2->glCompileShader(fragmentShaderTextRender);

	// Check for compile errors
	GLint fragmentShaderCompileStatusTextRender;
	libGl2->glGetShaderiv(fragmentShaderTextRender, GL_COMPILE_STATUS,
			&fragmentShaderCompileStatusTextRender);
	if (fragmentShaderCompileStatus != GL_TRUE) {
		char infoLog[512];
		libGl2->glGetShaderInfoLog(fragmentShaderTextRender, 512, NULL, infoLog);
		printf("Fragment shader compilation failed: %s\n", infoLog);
	}

	// Create program object
	programObject = libGl2->glCreateProgram();

	libGl2->glAttachShader(programObject, vertexShaderVncRender);
	libGl2->glAttachShader(programObject, fragmentShaderVncRender);
	libGl2->glLinkProgram(programObject);

	programObjectTextRender = libGl2->glCreateProgram();
	libGl2->glAttachShader(programObjectTextRender, vertexShaderTextRender);
	libGl2->glAttachShader(programObjectTextRender, fragmentShaderTextRender);
	libGl2->glLinkProgram(programObjectTextRender);

	// Check for linking errors
	GLint programLinkStatus;
	libGl2->glGetProgramiv(programObject, GL_LINK_STATUS, &programLinkStatus);
	if (programLinkStatus != GL_TRUE) {
		char infoLog[512];
		libGl2->glGetProgramInfoLog(programObject, 512, NULL, infoLog);
		printf("Program linking failed: %s\n", infoLog);
	}

	libGl2->glGetProgramiv(programObjectTextRender, GL_LINK_STATUS, &programLinkStatus);
	if (programLinkStatus != GL_TRUE) {
		char infoLog[512];
		libGl2->glGetProgramInfoLog(programObjectTextRender, 512, NULL, infoLog);
		printf("Program linking failed: %s\n", infoLog);
	}

	// Set clear color to black
	libGl2->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

}

char* parseFramebufferUpdate(int socket_fd, int* frameBufferWidth,
		int* frameBufferHeight, z_stream strm, int* finalHeight) {
	// Read message-type (1 byte) - not used, assuming it's always 0
	// Message type
	char messageType[1];
	if (!recv(socket_fd, messageType, 1, MSG_WAITALL)) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			std::cerr << "Receive timeout occurred while reading message type"
					<< std::endl;
		} else {
			std::cerr << "Error reading message type: " << strerror(errno)
					<< std::endl;
		}
		close(socket_fd);
		return NULL;
	}

	// Read padding (1 byte) - unused
	char padding[1];
	if (!recv(socket_fd, padding, 1, MSG_WAITALL)) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			std::cerr << "Receive timeout occurred while reading padding"
					<< std::endl;
		} else {
			std::cerr << "Error reading padding: " << strerror(errno)
					<< std::endl;
		}
		close(socket_fd);
		return NULL;
	}

	// Read number-of-rectangles (2 bytes)
	char numberOfRectangles[2];
	if (!recv(socket_fd, numberOfRectangles, 2, MSG_WAITALL)) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			std::cerr
					<< "Receive timeout occurred while reading number of rectangles"
					<< std::endl;
		} else {
			std::cerr << "Error reading number of rectangles: " << strerror(
					errno) << std::endl;
		}
		close(socket_fd);
		return NULL;
	}

	// Calculate the total size of the message
	int totalLoadedSize = 0; // message-type + padding + number-of-rectangles
	char* finalFrameBuffer = (char*) malloc(1);
	int offset = 0;
	int ret = 0;
	// Now parse each rectangle
	for (int i = 0; i < byteArrayToInt16(numberOfRectangles); i++) {
		// Read rectangle header
		char xPosition[2];
		char yPosition[2];
		char width[2];
		char height[2];
		char encodingType[4]; // S32
		char compressedDataSize[4]; // S32

		if (!recv(socket_fd, xPosition, 2, MSG_WAITALL) || !recv(socket_fd,
				yPosition, 2, MSG_WAITALL) || !recv(socket_fd, width, 2,
				MSG_WAITALL) || !recv(socket_fd, height, 2, MSG_WAITALL)
				|| !recv(socket_fd, encodingType, 4, MSG_WAITALL)) {
			fprintf(stderr, "Error reading rectangle header\n");
			return NULL;
		}

		*frameBufferWidth = byteArrayToInt16(width);
		*frameBufferHeight = byteArrayToInt16(height);
		*finalHeight = *finalHeight + *frameBufferHeight;
		if (encodingType[3] == '\x6') // ZLIB encoding
		{
			// Read zlib compressed data size with timeout handling
			if (!recv(socket_fd, compressedDataSize, 4, MSG_WAITALL)) {
				if (errno == EWOULDBLOCK || errno == EAGAIN) {
					std::cerr
							<< "Receive timeout occurred while reading zlib compressed data size"
							<< std::endl;
				} else {
					std::cerr << "Zlib compressed data size not found: "
							<< strerror(errno) << std::endl;
				}
				close(socket_fd);
				return NULL;
			}

			char* compressedData = (char*) malloc(byteArrayToInt32(
					compressedDataSize));

			int compresedDataReceivedSize = recv(socket_fd, compressedData,
					byteArrayToInt32(compressedDataSize), MSG_WAITALL);
			if (compresedDataReceivedSize < 0) {
				if (errno == EWOULDBLOCK || errno == EAGAIN) {
					std::cerr
							<< "Receive timeout occurred while receiving compressed data"
							<< std::endl;
				} else {
					perror("Error receiving compressed data");
				}
				free(compressedData);
				close(socket_fd);
				return NULL;
			}

			// Allocate memory for decompressed data (assuming it's at most the same size as compressed)
			char* decompressedData = (char*) malloc(*frameBufferWidth
					* *frameBufferHeight * 4);
			if (!decompressedData) {
				perror("Error allocating memory for decompressed data");
				free(decompressedData);
				close(socket_fd);
				return NULL;
			}

			// Resize finalFrameBuffer to accommodate the appended data
			totalLoadedSize = totalLoadedSize + (*frameBufferWidth
					* *frameBufferHeight * 4);
			finalFrameBuffer = (char*) realloc(finalFrameBuffer,
					totalLoadedSize);

			// Decompress the data
			strm.avail_in = compresedDataReceivedSize;
			strm.next_in = (Bytef*) compressedData;
			strm.avail_out = *frameBufferWidth * *frameBufferHeight * 4; // Use the actual size of the decompressed data
			strm.next_out = (Bytef*) decompressedData;

			ret = inflate(&strm, Z_NO_FLUSH);

			if (ret < 0 && ret != Z_BUF_ERROR) {
				fprintf(stderr, "Error: Failed to decompress zlib data: %s\n",
						strm.msg);
				inflateEnd(&strm);
				free(decompressedData);
				free(compressedData);
				close(socket_fd);
				return NULL;
			}

			memcpy(finalFrameBuffer + offset, decompressedData,
					*frameBufferWidth * *frameBufferHeight * 4);
			offset = offset + (*frameBufferWidth * *frameBufferHeight * 4);
			// Free memory allocated for framebufferUpdateRectangle
			free(compressedData);
			free(decompressedData);
		}
	}
	return finalFrameBuffer;
}
void print_string(float x, float y, const char* text, float r, float g,
		float b, float size) {
	char inputBuffer[2000] = { 0 }; // ~20s chars
	GLfloat triangleBuffer[2000] = { 0 };
	int number = stb_easy_font_print(0, 0, text, NULL, inputBuffer,
			sizeof(inputBuffer));

	// calculate movement inside viewport
	float ndcMovementX = (2.0f * x) / windowWidth;
	float ndcMovementY = (2.0f * y) / windowHeight;

	int triangleIndex = 0; // Index to keep track of the current position in the triangleBuffer
	// Convert each quad into two triangles and also apply size and offset to draw it to correct place
	for (int i = 0; i < sizeof(inputBuffer) / sizeof(GLfloat); i += 8) {
		// Triangle 1
		triangleBuffer[triangleIndex++]
				= *reinterpret_cast<GLfloat*> (&inputBuffer[i * sizeof(GLfloat)])
						/ size + ndcMovementX;
		triangleBuffer[triangleIndex++]
				= *reinterpret_cast<GLfloat*> (&inputBuffer[(i + 1)
						* sizeof(GLfloat)]) / size * -1 + ndcMovementY;
		triangleBuffer[triangleIndex++]
				= *reinterpret_cast<GLfloat*> (&inputBuffer[(i + 2)
						* sizeof(GLfloat)]) / size + +ndcMovementX;
		triangleBuffer[triangleIndex++]
				= *reinterpret_cast<GLfloat*> (&inputBuffer[(i + 3)
						* sizeof(GLfloat)]) / size * -1 + ndcMovementY;
		triangleBuffer[triangleIndex++]
				= *reinterpret_cast<GLfloat*> (&inputBuffer[(i + 4)
						* sizeof(GLfloat)]) / size + ndcMovementX;
		triangleBuffer[triangleIndex++]
				= *reinterpret_cast<GLfloat*> (&inputBuffer[(i + 5)
						* sizeof(GLfloat)]) / size * -1 + ndcMovementY;

		//// Triangle 2
		triangleBuffer[triangleIndex++]
				= *reinterpret_cast<GLfloat*> (&inputBuffer[i * sizeof(GLfloat)])
						/ size + ndcMovementX;
		triangleBuffer[triangleIndex++]
				= *reinterpret_cast<GLfloat*> (&inputBuffer[(i + 1)
						* sizeof(GLfloat)]) / size * -1 + ndcMovementY;
		triangleBuffer[triangleIndex++]
				= *reinterpret_cast<GLfloat*> (&inputBuffer[(i + 4)
						* sizeof(GLfloat)]) / size + ndcMovementX;
		triangleBuffer[triangleIndex++]
				= *reinterpret_cast<GLfloat*> (&inputBuffer[(i + 5)
						* sizeof(GLfloat)]) / size * -1 + ndcMovementY;
		triangleBuffer[triangleIndex++]
				= *reinterpret_cast<GLfloat*> (&inputBuffer[(i + 6)
						* sizeof(GLfloat)]) / size + ndcMovementX;
		triangleBuffer[triangleIndex++]
				= *reinterpret_cast<GLfloat*> (&inputBuffer[(i + 7)
						* sizeof(GLfloat)]) / size * -1 + ndcMovementY;

	}

	libGl2->glUseProgram(programObjectTextRender);
	GLuint vbo;
	libGl2->glGenBuffers(1, &vbo);
	libGl2->glBindBuffer(GL_ARRAY_BUFFER, vbo);
	libGl2->glBufferData(GL_ARRAY_BUFFER, sizeof(triangleBuffer), triangleBuffer,
			GL_STATIC_DRAW);

	// Specify the layout of the vertex data
	GLint positionAttribute = libGl2->glGetAttribLocation(programObject, "position");
	libGl2->glEnableVertexAttribArray(positionAttribute);
	libGl2->glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	// glEnableVertexAttribArray(0);

	// Render the triangle
	libGl2->glDrawArrays(GL_TRIANGLES, 0, triangleIndex);

	libGl2->glDeleteBuffers(1, &vbo);
}

void print_string_center(float y, const char* text, float r, float g, float b,
		float size) {
	print_string(-stb_easy_font_width(text) * (size / 200), y, text, r, g, b,
			size);
}

// Helper function to parse a line for GLfloat arrays
void parseLineArray(char *line, const char *key, GLfloat *dest, int count) {
    if (strncmp(line, key, strlen(key)) == 0) {
        char *values = strchr(line, '=');
        if (values) {
            values++; // Skip '='
            for (int i = 0; i < count; i++) {
                dest[i] = strtof(values, &values); // Parse floats
            }
        }
    }
}

// Helper function to parse a line for integers
void parseLineInt(char *line, const char *key, int *dest) {
    if (strncmp(line, key, strlen(key)) == 0) {
        char *value = strchr(line, '=');
        if (value) {
            *dest = atoi(value + 1); // Parse integer
        }
    }
}

// Function to load the configuration file
void loadConfig(const char *filename) {
	FILE *file = fopen(filename, "r");
	if (!file) {
		printf("Config file not found. Using defaults.\n");
		return;
	}

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        parseLineArray(line, "landscapeVertices", landscapeVertices, 12);
        parseLineArray(line, "portraitVertices", portraitVertices, 12);
        parseLineArray(line, "landscapeTexCoords", landscapeTexCoords, 8);
        parseLineArray(line, "portraitTexCoords", portraitTexCoords, 8);
        parseLineInt(line, "windowWidth", &windowWidth);
        parseLineInt(line, "windowHeight", &windowHeight);
    }

	fclose(file);
}

// Function to print GLfloat arrays
void printArray(const char *label, GLfloat *array, int count, int elementsPerLine) {
    printf("%s:\n", label);
    for (int i = 0; i < count; i++) {
        printf("%f ", array[i]);
        if ((i + 1) % elementsPerLine == 0) printf("\n");
    }
    printf("\n");
}

#endif
