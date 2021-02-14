
#include <stdarg.h>
#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void Verbose(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, fmt, ap);
	va_end(ap);
}

#ifdef __cplusplus
}
#endif
