#include "vulkun.h"

#if (defined(unix) || defined(__APPLE__))
#define IS_UNIX
#endif

#ifdef IS_UNIX
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

void handle_segfault(int sig) {
	const size_t max_frames = 63;
	// Get stack frame addresses
	void *array[max_frames];
	size_t size = backtrace(array, max_frames);

	// Print stack trace information
	fprintf(stderr, "Error: Signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(sig);
}
#endif

#ifdef _WIN32
int wmain() {
#else
int main() {
#endif
#ifdef IS_UNIX
	// Register signal handler
	signal(SIGSEGV, handle_segfault);
#endif

	Vulkun vulkun;
	vulkun.init();
	if (vulkun.is_initialized()) {
		vulkun.run();
	}
	vulkun.cleanup();

	return 0;
}
