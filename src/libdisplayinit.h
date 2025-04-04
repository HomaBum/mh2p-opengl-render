#include <screen/screen.h>

#define DINT_LIB_NAME "libdisplayinit.so"

class LibDisplayInit {

	public:
		struct dint_screen_window
		{
			screen_window_t native_window;
			int nBuffers;
		};

	private:
		void* _func_handle;

		typedef int (*dint_init_handle)();
		typedef int (*dint_create_window_handle)(int isOverlayWithVideo, int display_id, int width, int height, int *properties, dint_screen_window **window);
		typedef int (*dint_get_native_window_handle)(dint_screen_window *window, screen_window_t *native_window_arg);

	protected:
		static LibDisplayInit* _singleton;

		LibDisplayInit();
		~LibDisplayInit();

	public:
		LibDisplayInit(LibDisplayInit &other) = delete;
		void operator=(const LibDisplayInit &) = delete;
		static LibDisplayInit *GetInstance();

		dint_init_handle dint_init;
		dint_create_window_handle dint_create_window;
		dint_get_native_window_handle dint_get_native_window;

};
