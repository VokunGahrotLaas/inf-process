#pragma once

// inf
#include <inf/errno_guard.hpp>

#ifndef _MSC_VER
#	define INF_GNU_PURE [[gnu::pure]]
#else
#	define INF_GNU_PURE
#endif

#ifndef _WIN32
#	include <unistd.h>
#	define INF_FDOPEN ::fdopen
#	define INF_WFDOPEN ::fdopen
#	define INF_DUP ::dup
#	define INF_DUP2 ::dup2
#	define INF_CLOSE ::close
#else
#	include <io.h>
#	include <fcntl.h>
#	include <windows.h>
#	define INF_FDOPEN ::_fdopen
#	define INF_WFDOPEN ::_wfdopen
#	define INF_DUP ::_dup
#	define INF_DUP2 ::_dup2
#	define INF_CLOSE ::_close
#endif

#ifdef _WIN32
namespace inf
{

inline int winhandle_to_fd(HANDLE handle, int flags, inf::source_location location = inf::source_location::current())
{
	errno_guard errg{ "_open_osfhandle" };
	int fd = ::_open_osfhandle(reinterpret_cast<intptr_t>(handle), flags);
	if (fd < 0) errg.throw_error(location);
	return fd;
}

inline HANDLE fd_to_winhandle(int fd, inf::source_location location = inf::source_location::current())
{
	errno_guard errg{ "_get_osfhandle" };
	HANDLE handle = reinterpret_cast<HANDLE>(::_get_osfhandle(fd));
	if (handle == INVALID_HANDLE_VALUE) errg.throw_error(location);
	return handle;
}

} // namespace inf
#endif
