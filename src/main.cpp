#include "vulkun.h"

int main() {
	Vulkun vulkun;
	vulkun.init();
	if (vulkun.is_initialized()) {
		vulkun.run();
	}
	vulkun.cleanup();

	return 0;
}
