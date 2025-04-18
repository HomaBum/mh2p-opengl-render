//#include "opengl.hh"
#include "libdisplayinit.h"
#include "RenderManager.h"
#include <string>

bool running = true;

#define SHMEM_ENABLE 1
#if SHMEM_ENABLE
#include "SharedMemoryReader.hh"
#endif

void stopHandler(int param)
{
  if ( param == SIGINT )
    running = false;
}

struct TurnInfo {
	char road[255];
    int turnSide;
    int event;
    int turnAngle;
    int turnNumber;
    int validFlag;
    int distanceMeters;
    int timeSeconds;

    void dump() {
        printf("=== TurnInfo Dump ===\n");
        printf("Road:       %s\n", road);
        printf("Turn Side:    %d\n", turnSide);
        printf("Event:        %d\n", event);
        printf("Turn Angle:   %d°\n", turnAngle);
        printf("Turn Number:  %d\n", turnNumber);
        printf("Valid Flag:   %d\n", validFlag);
        printf("Distance:     %d meters\n", distanceMeters);
        printf("Time:         %d seconds\n", timeSeconds);
        printf("=====================\n");
    }
};

int main(int argc, char* argv[]) {

	printf("Q3 F3 Team OpenGL render 0.0.1 (QNX MOST VNC render 0.1.0 fork)\n");

	int context = 20;
	int width = 858;
	int height = 322;

	if(argc == 2 && strstr(argv[1], "-h")) {
		printf("Usage: ./mh2p-opengl-render [context] [width] [height]\n");
		printf("Default: context = %d, width = %d, height = %d \n", context, width, height);
		return 0;
	}

	if(argc >= 2) {
		context = std::stoul(argv[1], 0, 10);
	}
	if(argc >= 3) {
		width = std::stoul(argv[2], 0, 10);
	}
	if(argc >= 4) {
		height = std::stoul(argv[3], 0, 10);
	}
	RenderManager renderManager(context, width, height);
	printf("RenderManager inited \n");

	signal(SIGINT, stopHandler);

	std::wstring renderStr = L"Test флапа кокпита";
	renderManager.render(renderStr);
	printf("Call first render \n");

	SharedMemoryReader* shmemReader;
#if SHMEM_ENABLE
	try {
		shmemReader = new SharedMemoryReader();
	}
	catch(std::runtime_error& e) {
		printf("ERROR: SharedMemory(): %s\n", e.what());
	}
	catch(...) {
		printf("ERROR: SharedMemory()\n");
	}
	printf("SharedMemoryReader inited \n");
	TurnInfo turnInfo;
#endif

	while(running) {
#if SHMEM_ENABLE
		if(shmemReader)
		{
			try {
				shmemReader->read(&turnInfo, 0, sizeof(turnInfo));
				printf("shmemReader read \n");
				turnInfo.dump();
				renderManager.render(turnInfo.road);
				printf("renderManager.render() ok \n");
			}
			catch(std::runtime_error& e) {
				printf("ERROR: SharedMemory.read(): %s\n", e.what());
				renderManager.render(renderStr);
			}
			catch(...) {
				printf("ERROR: SharedMemory.read()\n");
				renderManager.render(renderStr);
			}
		}
		else
		{
			printf("SharedMemoryReader is null\n");
			renderManager.render(renderStr);
		}
#else
		renderManager.render(renderStr);
#endif
	}

	if(shmemReader) {
		delete shmemReader;
	}

	return 0;
}
