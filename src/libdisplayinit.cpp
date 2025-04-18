#include "libdisplayinit.h"
#include <dlfcn.h>
#include <stdlib.h>

LibDisplayInit* LibDisplayInit::_singleton= nullptr;;

#define LOAD_DINT_FUNC(func) \
    this->func = (func##_handle)dlsym(this->_func_handle, #func); \
    if (!this->func) { \
        printf("Error while loading %s function %s: %s\n", DINT_LIB_NAME, #func, dlerror()); \
        exit(EXIT_FAILURE); \
    }

LibDisplayInit::LibDisplayInit()
{
	printf("LibDisplayInit start\n");

	this->_func_handle = 0;

	this->_func_handle = dlopen(DINT_LIB_NAME, RTLD_NOW);
	if (!this->_func_handle) {
		printf("Error using %s: %s\n", DINT_LIB_NAME, dlerror());
		exit(EXIT_FAILURE);
	}

	LOAD_DINT_FUNC(dint_init)
	LOAD_DINT_FUNC(dint_create_window)
	LOAD_DINT_FUNC(dint_get_native_window)
	LOAD_DINT_FUNC(dint_deinit)
	LOAD_DINT_FUNC(dint_destroy_window)

	printf("LibDisplayInit successful\n");
}

LibDisplayInit::~LibDisplayInit()
{
	printf("~LibDisplayInit start\n");

	if(this->_func_handle)
	{
		if(dlclose(this->_func_handle) != 0)
		{
			printf("Error while closing %s handle: %s\n", DINT_LIB_NAME, dlerror());
		}
	}

	printf("~LibDisplayInit successful\n");
}

LibDisplayInit* LibDisplayInit::GetInstance()
{
    if(_singleton == nullptr){
    	_singleton = new LibDisplayInit();
    }
    return _singleton;
}
