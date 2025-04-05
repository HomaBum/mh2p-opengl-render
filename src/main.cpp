#include "opengl.hh"
#include "libdisplayinit.h"

int main(int argc, char* argv[]) {

	printf("Q3 F3 Team OpenGL render 0.0.1 (QNX MOST VNC render 0.1.0 fork)\n");

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
    screen_window_t native_window = 0;

	int result = 0;

	// Call dint_init()
	result = libDisplayInit->dint_init();
	printf("dint_init result = %d\n", result);

	// Call dint_create_window()
	int display_id = 0;
	int properties[3];
	properties[0] = 5;
	properties[1] = 1;
	properties[2]= 0;
	result = libDisplayInit->dint_create_window(0, display_id, windowWidth, windowHeight, properties, &dint_window);
	if(result) {
		printf("Failed to create window %d!!!\n", result);
		exit(EXIT_FAILURE);
	}
	printf("dint_create_window result = %d, native_window = %p, nBuffers = %d\n", result, dint_window->native_window, dint_window->nBuffers);

	// Call dint_get_native_window()
	result = libDisplayInit->dint_get_native_window(dint_window, &native_window);
	if(result) {
		printf("Failed to get native window %d!!!\n", result);
		exit(EXIT_FAILURE);
	}
	printf("dint_get_native_window result = %d, native_window = %p\n", result, native_window);
	// Init Screen Done

	// Init EGL
	printf("OpenGL ES2.0 initialization started \n");
	eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY); // DONE
	if(eglDisplay == EGL_NO_DISPLAY) {
		printf("ERROR: no EGL Display\n");
		exit(EXIT_FAILURE);
	}
	printf("eglGetDisplay %p\n", eglDisplay);
	int major = 0, minor = 0;
	if(eglInitialize(eglDisplay, &major, &minor) == EGL_FALSE) {
		printf("Error eglInitialize!!!\n");
		exit(EXIT_FAILURE);
	}
	printf("eglInitialize: Version %d.%d\n", major, minor);

	EGLint config_attribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RED_SIZE,
				1, EGL_GREEN_SIZE, 1, EGL_BLUE_SIZE, 1, EGL_ALPHA_SIZE, 1,
				EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE };
	EGLint num_configs;
	eglChooseConfig(eglDisplay, config_attribs, 0, 0, &num_configs);
	printf("eglChooseConfig: num_configs = %d\n", num_configs);

	if(num_configs < 1) {
		printf("Error: no configs!!!\n");
		exit(EXIT_FAILURE);
	}

	EGLConfig* configs = new EGLConfig[num_configs];
	eglChooseConfig(eglDisplay, config_attribs, configs, num_configs, &num_configs);
	printf("eglChooseConfig: configs = %p, num_configs = %d\n", configs, num_configs);

	EGLSurface eglSurface = EGL_NO_SURFACE;
	EGLConfig eglConfig = 0;
	for(int i = 0; i < num_configs; ++i) {
		printf("OpenGLES: eglCreateWindowSurface with %d config \n", i);
		EGLNativeWindowType windowEgl = native_window;
		eglSurface = eglCreateWindowSurface(eglDisplay, configs[i], windowEgl, 0);
		if(eglSurface) {
			eglConfig = configs[i];
			break;
		}
		checkErrorEGL("eglCreateWindowSurface");
		printf("Create surface failed: 0x%x\n", eglSurface);
	}

	if(eglSurface == EGL_NO_SURFACE) {
		printf("Error: no eglSurface!!!\n");
		exit(EXIT_FAILURE);
	}


	printf("OpenGLES: eglBindAPI \n");
	if(eglBindAPI(EGL_OPENGL_ES_API) == EGL_FALSE) {
		printf("Error eglBindAPI!!!\n");
		checkErrorEGL("eglBindAPI");
		exit(EXIT_FAILURE);
	}

	const EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	printf("OpenGLES: eglCreateContext \n");
	eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, context_attribs);
	checkErrorEGL("eglCreateContext");
	if (eglContext == EGL_NO_CONTEXT) {
		std::cerr << "Failed to create EGL context" << std::endl;
		return EXIT_FAILURE;
	}

	printf("OpenGLES: eglMakeCurrent \n");
	EGLBoolean madeCurrent = eglMakeCurrent(eglDisplay, eglSurface, eglSurface,
			eglContext);
	if (madeCurrent == EGL_FALSE) {
		std::cerr << "Failed to make EGL context current" << std::endl;
		return EXIT_FAILURE;
	}

	printf("First part successful \n");


	// Initialize OpenGL ES
	Init();

	//test

	while(true) {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(programObjectTextRender);
        print_string(-333, 160, "Q3 F3 Team OpenGL", 0.5, 0.5, 0.5, 150);
        eglSwapBuffers(eglDisplay, eglSurface);
	}

	while (true) {
		printf("Main loop executed \n");
		execute_final_commands();
		int sockfd;
		fd_set write_fds;
		int result;
		int so_error;
		socklen_t len = sizeof(so_error);
		struct sockaddr_in serv_addr;

		int keepalive = 1; // Enable keepalive
		int keepidle = 2; // Idle time before sending the first keepalive probe (in seconds)

		// Create socket
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) {
			perror("Error opening socket");
			close(sockfd);
			continue;
		}

		int mib[4];
		int ival = 0;

		mib[0] = CTL_NET;
		mib[1] = AF_INET;
		mib[2] = IPPROTO_TCP;
		mib[3] = TCPCTL_KEEPCNT;
		ival = 3;
		sysctl(mib, 4, NULL, NULL, &ival, sizeof(ival));

		mib[0] = CTL_NET;
		mib[1] = AF_INET;
		mib[2] = IPPROTO_TCP;
		mib[3] = TCPCTL_KEEPINTVL;
		ival = 2;
		sysctl(mib, 4, NULL, NULL, &ival, sizeof(ival));

		// Enable TCP keepalive
		if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive,
				sizeof(keepalive)) < 0) {
			perror("setsockopt SO_KEEPALIVE");
			close(sockfd);
			continue;
		}

		// Set the time (in seconds) the connection needs to remain idle before TCP starts sending keepalive probes
		if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPALIVE, &keepidle,
				sizeof(keepidle)) < 0) {
			perror("setsockopt TCP_KEEPIDLE");
			close(sockfd);
			continue;
		}

		printf("TCP keepalive enabled and configured on the socket.\n");

		// Set socket to non-blocking mode
		int flags = fcntl(sockfd, F_GETFL, 0);
		if (flags < 0) {
			perror("fcntl F_GETFL");
			close(sockfd);
			continue;
		}
		if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
			perror("fcntl F_SETFL");
			close(sockfd);
			continue;
		}

		// Initialize socket structure
		memset((char*) &serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		if (argc > 1) {
			// Use IP address from command line argument
			serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
		} else {
			// Fallback to default IP address if no argument is provided
			serv_addr.sin_addr.s_addr = inet_addr(VNC_SERVER_IP_ADDRESS);
		}
		serv_addr.sin_port = htons(VNC_SERVER_PORT);

		struct timeval timeout;
		timeout.tv_sec = 10; // 10 seconds timeout read/write
		timeout.tv_usec = 0;

		// Set receive timeout
		if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout,
				sizeof(timeout)) < 0) {
			perror("Set receive timeout failed");
			close(sockfd);
			continue;
		}

		// Set send timeout
		if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*) &timeout,
				sizeof(timeout)) < 0) {
			perror("Set send timeout failed");
			close(sockfd);
			continue;
		}

		result = connect(sockfd, (struct sockaddr*) &serv_addr,
				sizeof(serv_addr));
		if (result < 0 && errno != EINPROGRESS) {
			perror("Error connecting to server");
			usleep(200000); // Sleep for 200 milliseconds (200,000 microseconds)
			close(sockfd);
			continue;
		}

		// Initialize file descriptor set
		FD_ZERO(&write_fds);
		FD_SET(sockfd, &write_fds);

		// Set timeout value
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		// Wait for the socket to be writable (connection established)
		result = select(sockfd + 1, NULL, &write_fds, NULL, &timeout);
		if (result < 0) {
			perror("select failed");
			close(sockfd);
			usleep(200000); // Sleep for 200 milliseconds (200,000 microseconds)
			continue;
		} else if (result == 0) {
			printf("Connection timed out\n");
			close(sockfd);
			usleep(200000); // Sleep for 200 milliseconds (200,000 microseconds)
			continue;
		} else {
			// Check if the connection was successful
			if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
				perror("getsockopt");
				close(sockfd);
				usleep(200000); // Sleep for 200 milliseconds (200,000 microseconds)
				continue;
			}
			if (so_error == 0) {
				printf("Connected to the server: %s\n", inet_ntoa(
						serv_addr.sin_addr));
			} else {
				printf("Connection failed: %s\n", strerror(so_error));
				usleep(200000); // Sleep for 200 milliseconds (200,000 microseconds)
				close(sockfd);
				continue;
			}
		}

		// Reset socket to blocking mode
		if (fcntl(sockfd, F_SETFL, flags) < 0) {
			perror("fcntl F_SETFL");
			usleep(200000); // Sleep for 200 milliseconds (200,000 microseconds)
			close(sockfd);
			continue;
		}

		if (sockfd != NULL) {
			// we have connection so swap to different view
			execute_initial_commands();
		}

		// Receive server initialization message
		char serverInitMsg[12];
		int bytesReceived = recv(sockfd, serverInitMsg, sizeof(serverInitMsg),
				MSG_WAITALL);
		if (bytesReceived < 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				std::cerr << "Receive timeout occurred" << std::endl;
			} else {
				std::cerr << "Error receiving server initialization message: "
						<< strerror(errno) << std::endl;
			}
			close(sockfd);
			continue;
		} else if (bytesReceived == 0) {
			std::cerr << "Connection closed by peer" << std::endl;
			close(sockfd);
			continue;
		}

		// Send client protocol version message with timeout handling
		if (send(sockfd, PROTOCOL_VERSION, strlen(PROTOCOL_VERSION), 0) < 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				std::cerr << "Send timeout occurred" << std::endl;
			} else {
				std::cerr << "Error sending client initialization message: "
						<< strerror(errno) << std::endl;
			}
			close(sockfd);
			continue;
		}
		// Security handshake
		char securityHandshake[4];
		ssize_t bytesReceivedSecurity = recv(sockfd, securityHandshake,
				sizeof(securityHandshake), 0);
		if (bytesReceivedSecurity < 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				std::cerr << "Receive timeout occurred" << std::endl;
			} else {
				std::cerr << "Error reading security handshake: " << strerror(
						errno) << std::endl;
			}
			close(sockfd);
			continue;
		} else if (bytesReceivedSecurity == 0) {
			std::cerr << "Connection closed by peer" << std::endl;
			close(sockfd);
			continue;
		}

		if (send(sockfd, "\x01", 1, 0) < 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				std::cerr << "Send timeout occurred" << std::endl;
			} else {
				std::cerr << "Error sending client init message: " << strerror(
						errno) << std::endl;
			}
			close(sockfd);
			continue;
		}

		// Read framebuffer width and height
		char framebufferWidth[2];
		char framebufferHeight[2];

		ssize_t bytesReceivedFrameBuffer = recv(sockfd, framebufferWidth,
				sizeof(framebufferWidth), 0);
		if (bytesReceivedFrameBuffer < 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				std::cerr
						<< "Receive timeout occurred while reading framebuffer width"
						<< std::endl;
			} else {
				std::cerr << "Error reading framebuffer width: " << strerror(
						errno) << std::endl;
			}
			close(sockfd);
			continue;
		} else if (bytesReceivedFrameBuffer == 0) {
			std::cerr
					<< "Connection closed by peer while reading framebuffer width"
					<< std::endl;
			close(sockfd);
			continue;
		}

		bytesReceivedFrameBuffer = recv(sockfd, framebufferHeight,
				sizeof(framebufferHeight), 0);
		if (bytesReceivedFrameBuffer < 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				std::cerr
						<< "Receive timeout occurred while reading framebuffer height"
						<< std::endl;
			} else {
				std::cerr << "Error reading framebuffer height: " << strerror(
						errno) << std::endl;
			}
			close(sockfd);
			continue;
		} else if (bytesReceivedFrameBuffer == 0) {
			std::cerr
					<< "Connection closed by peer while reading framebuffer height"
					<< std::endl;
			close(sockfd);
			continue;
		}

		// Read pixel format and name length
		char pixelFormat[16];
		char nameLength[4];

		if (!recv(sockfd, pixelFormat, sizeof(pixelFormat), MSG_WAITALL)
				|| !recv(sockfd, nameLength, sizeof(nameLength), MSG_WAITALL)) {
			fprintf(stderr, "Error reading pixel format or name length\n");
			close(sockfd);
			continue;
		}

		uint32_t nameLengthInt = (nameLength[0] << 24) | (nameLength[1] << 16)
				| (nameLength[2] << 8) | nameLength[3];

		// Read server name
		char name[32];
		if (!recv(sockfd, name, nameLengthInt, MSG_WAITALL)) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				std::cerr
						<< "Receive timeout occurred while reading server name"
						<< std::endl;
			} else {
				std::cerr << "Error reading server name: " << strerror(errno)
						<< std::endl;
			}
			close(sockfd);
			continue;
		}

		// Send encoding update requests
		if (send(sockfd, ZLIB_ENCODING, sizeof(ZLIB_ENCODING), 0) < 0 || send(
				sockfd, FRAMEBUFFER_UPDATE_REQUEST,
				sizeof(FRAMEBUFFER_UPDATE_REQUEST), 0) < 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				std::cerr
						<< "Send timeout occurred while sending framebuffer update request"
						<< std::endl;
			} else {
				std::cerr << "Error sending framebuffer update request: "
						<< strerror(errno) << std::endl;
			}
			close(sockfd);
			continue;
		}
		int framebufferWidthInt = 0;
		int framebufferHeightInt = 0;
		int finalHeight = 0;

		int frameCount = 0;
		double fps = 0.0;
		time_t startTime = time(NULL);

		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Set texture parameters (optional)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Set texture wrapping mode (optional)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;

		int ret = inflateInit(&strm);
		if (ret != Z_OK) {
			fprintf(stderr, "Error: Failed to initialize zlib decompression\n");
			continue;
		}

		// Main loop
		execute_initial_commands();
		while (true) {
			frameCount++;
			char* framebufferUpdate = parseFramebufferUpdate(sockfd,
					&framebufferWidthInt, &framebufferHeightInt, strm,
					&finalHeight);
			if (framebufferUpdate == NULL) {
				close(sockfd);
				break;
			}

			// Send framebuffer update request with timeout handling
			if (send(sockfd, FRAMEBUFFER_UPDATE_REQUEST,
					sizeof(FRAMEBUFFER_UPDATE_REQUEST), 0) < 0) {
				if (errno == EWOULDBLOCK || errno == EAGAIN) {
					std::cerr
							<< "Send timeout occurred while sending framebuffer update request"
							<< std::endl;
				} else {
					std::cerr << "Error sending framebuffer update request: "
							<< strerror(errno) << std::endl;
				}
				close(sockfd);
				break;
			}

			// Calculate elapsed time
			time_t currentTime = time(NULL);
			double elapsedTime = difftime(currentTime, startTime);

			// Calculate FPS every second
			if (elapsedTime >= 1.0) {
				// Calculate FPS
				fps = frameCount / elapsedTime;

				// Reset frame count and start time
				frameCount = 0;
				startTime = currentTime;
			}
			glClear(GL_COLOR_BUFFER_BIT); // clear all
			glUseProgram(programObject);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebufferWidthInt,
					finalHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
					framebufferUpdate);

			// Set vertex positions
			GLint positionAttribute = glGetAttribLocation(programObject,
					"position");
			if (framebufferWidthInt > finalHeight)
				glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
						0, landscapeVertices);
			else
				glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
						0, portraitVertices);

			glEnableVertexAttribArray(positionAttribute);

			// Set texture coordinates
			GLint texCoordAttrib = glGetAttribLocation(programObject,
					"texCoord");
			if (framebufferWidthInt > finalHeight)
				glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 0,
						landscapeTexCoords);
			else
				glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 0,
						portraitTexCoords);

			glEnableVertexAttribArray(texCoordAttrib);
			finalHeight = 0;
			// Draw quad
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

			//print_string(-333, 150, readPersistanceData("s:2001:101").c_str(), 1, 1, 1, 64); // persistance data

			eglSwapBuffers(eglDisplay, eglSurface);
			free(framebufferUpdate); // Free the dynamically allocated memory
		}
		glDeleteTextures(1, &textureID);
		execute_final_commands();
	}
	// Cleanup
	eglSwapBuffers(eglDisplay, eglSurface);
	eglDestroySurface(eglDisplay, eglSurface);
	eglDestroyContext(eglDisplay, eglContext);
	eglTerminate(eglDisplay);
	execute_final_commands();

	return EXIT_SUCCESS;
}
