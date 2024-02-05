#include "vulkun.h"

int main() {
	Vulkun vulkun;
	vulkun.init();
	vulkun.run();
	vulkun.cleanup();

	return 0;
}
