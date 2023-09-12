#pragma once

// STL
#include <string>
//
#ifndef _WIN32
#	include <spawn.h>
#	include <sys/wait.h>
#else
#	include <process.h>
#endif
// inf
#include <inf/with_errno.hpp>

#ifndef _WIN32
extern char** environ;
#endif

namespace inf
{

class spawn
{
public:
	spawn(char const* path, char* const* argv, char* const* envp,
		  inf::source_location location = inf::source_location::current())
#ifndef _WIN32
		: pid_{ -1 }
#else
		: handle_{ -1 }
#endif
	{
#ifndef _WIN32
		if (envp == nullptr) envp = ::environ;
		if (with_errno werr{ "posix_spawnp" }; ::posix_spawnp(&pid_, path, nullptr, nullptr, argv, envp) != 0)
			werr.throw_error(location);
#else
		if (with_errno werr{ "posix_spawnp" }; (handle_ = ::_spawnvpe(_P_NOWAIT, path, argv, envp)) == -1)
			werr.throw_error(location);
#endif
	}

#ifndef _WIN32
	pid_t pid() const { return pid_; }
#else
	intptr_t handle() const { return handle_; }
#endif

	int wait(inf::source_location location = inf::source_location::current())
	{
		int status = -1;
#ifndef _WIN32
		if (pid_ <= 0) return -1;
		if (with_errno werr{ "waitpid" })
		{
			pid_t res = ::waitpid(pid_, &status, 0);
			if (res < 0) werr.throw_error(location);
		}
#else
		if (handle_ == -1) return -1;
		if (with_errno werr{ "_cwait" })
		{
			pid_t res = ::_cwait(&status, handle_, _WAIT_CHILD);
			if (res == -1) werr.throw_error(location);
		}
#endif
		return status;
	}

private:
#ifndef _WIN32
	pid_t pid_;
#else
	intptr_t handle_;
#endif
};

} // namespace inf
