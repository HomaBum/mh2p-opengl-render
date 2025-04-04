#include "libdisplayinit.h"
#include <dlfcn.h>
#include <stdlib.h>

LibDisplayInit* LibDisplayInit::_singleton= nullptr;;

LibDisplayInit::LibDisplayInit()
{
	printf("LibDisplayInit start");

	this->_func_handle = 0;

	this->_func_handle = dlopen(DINT_LIB_NAME, RTLD_LAZY);
	if (!this->_func_handle) {
		printf("Error using %s: %s\n", DINT_LIB_NAME, dlerror());
		exit(EXIT_FAILURE);
	}

	this->dint_init = (dint_init_handle)dlsym(this->_func_handle, "dint_init");
	if (!this->dint_init) {
		printf("Error while loading %s function dint_init: %s\n", DINT_LIB_NAME, dlerror());
		exit(EXIT_FAILURE);
	}

	this->dint_create_window = (dint_create_window_handle)dlsym(this->_func_handle, "dint_create_window");
	if (!this->dint_create_window) {
		printf("Error while loading %s function dint_create_window: %s\n", DINT_LIB_NAME, dlerror());
		exit(EXIT_FAILURE);
	}

	this->dint_get_native_window = (dint_get_native_window_handle)dlsym(this->_func_handle, "dint_get_native_window");
	if (!this->dint_get_native_window) {
		printf("Error while loading %s function dint_get_native_window: %s\n", DINT_LIB_NAME, dlerror());
		exit(EXIT_FAILURE);
	}

	printf("LibDisplayInit successful");
}

LibDisplayInit::~LibDisplayInit()
{
	printf("~LibDisplayInit start");

	if(this->_func_handle)
	{
		if(dlclose(this->_func_handle) != 0)
		{
			printf("Error while closing %s handle: %s\n", DINT_LIB_NAME, dlerror());
		}
	}

	printf("~LibDisplayInit successful");
}

LibDisplayInit* LibDisplayInit::GetInstance()
{
    if(_singleton == nullptr){
    	_singleton = new LibDisplayInit();
    }
    return _singleton;
}
