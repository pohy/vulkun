#include "vulkun.h"

#ifdef _WIN32
int wmain() {
#else
int main() {
#endif
	Vulkun vulkun;
	vulkun.init();
	if (vulkun.is_initialized()) {
		vulkun.run();
	}
	vulkun.cleanup();

	return 0;
}
