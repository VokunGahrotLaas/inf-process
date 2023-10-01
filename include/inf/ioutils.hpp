#pragma once

// STL
#include <version>
// inf
#include <inf/errno_guard.hpp>
#include <inf/utils.hpp>

#ifndef _WIN32
#	include <unistd.h>
#	define INF_FDOPEN ::fdopen
#	define INF_WFDOPEN ::fdopen
#	define INF_DUP ::dup
#	define INF_DUP2 ::dup2
#	define INF_IO_CLOSE ::close
#	define INF_GETPID ::getpid
#else
#	include <io.h>
#	include <fcntl.h>
#	include <windows.h>
#	define INF_FDOPEN ::_fdopen
#	define INF_WFDOPEN ::_wfdopen
#	define INF_DUP ::_dup
#	define INF_DUP2 ::_dup2
#	define INF_IO_CLOSE ::_close
#	define INF_GETPID ::_getpid
#endif

namespace inf
{

namespace io
{

inline void close(int fd, source_location location = source_location::current())
{
	errno_guard errg{ INF_STRX(INF_IO_CLOSE) };
	if (INF_IO_CLOSE(fd) < 0) errg.throw_error(location);
}

inline int getpid(source_location = source_location::current()) { return INF_GETPID(); }

#ifdef _WIN32

inline void close_handle(HANDLE handle, source_location location = source_location::current())
{
	errno_guard errg{ "::CloseHandle" };
	if (!::CloseHandle(handle)) errg.throw_error(location);
}

inline int winhandle_to_fd(HANDLE handle, int flags, source_location location = source_location::current())
{
	errno_guard errg{ "_open_osfhandle" };
	int fd = ::_open_osfhandle(reinterpret_cast<intptr_t>(handle), flags);
	if (fd < 0) errg.throw_error(location);
	return fd;
}

inline HANDLE fd_to_winhandle(int fd, source_location location = source_location::current())
{
	errno_guard errg{ "_get_osfhandle" };
	HANDLE handle = reinterpret_cast<HANDLE>(::_get_osfhandle(fd));
	if (handle == INVALID_HANDLE_VALUE) errg.throw_error(location);
	return handle;
}

#endif

} // namespace io

} // namespace inf
