#pragma once

// inf
#include <inf/errno_guard.hpp>

#define INF_STR(X) #X
#define INF_STRX(X) INF_STR(X)

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
#	define INF_IO_CLOSE ::close
#else
#	include <io.h>
#	include <fcntl.h>
#	include <windows.h>
#	define INF_FDOPEN ::_fdopen
#	define INF_WFDOPEN ::_wfdopen
#	define INF_DUP ::_dup
#	define INF_DUP2 ::_dup2
#	define INF_IO_CLOSE ::_close
#endif

namespace inf
{

namespace io
{

inline void close(int fd, inf::source_location location = inf::source_location::current())
{
	errno_guard errg{ INF_STRX(INF_IO_CLOSE) };
	if (INF_IO_CLOSE(fd) < 0) errg.throw_error(location);
}

} // namespace io

#ifdef _WIN32

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
#endif

} // namespace inf
