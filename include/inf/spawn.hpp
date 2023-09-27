#pragma once

// STL
#include <cstdlib>
#include <functional>
#include <string>
// Unix / Windows
#ifndef _WIN32
#	include <spawn.h>
#	include <sys/wait.h>
#else
#	include <process.h>
#endif
// inf
#include <inf/errno_guard.hpp>
#include <inf/exceptions.hpp>

#ifndef _WIN32
extern char** environ;
#endif

namespace inf
{

template <int expected>
std::function<bool(int)> status_equals = [](int status) -> bool { return status == expected; };

class spawn
{
public:
	spawn(char const* path, char* const* argv, char* const* envp, source_location location = source_location::current())
#ifndef _WIN32
		: pid_{ -1 }
#else
		: handle_{ -1 }
#endif
	{
#ifndef _WIN32
		if (envp == nullptr) envp = ::environ;
		errno_guard errg{ "posix_spawnp" };
		int res = ::posix_spawnp(&pid_, path, nullptr, nullptr, argv, envp);
		if (res != 0) errg.throw_error(location);
#else
		errno_guard errg{ "posix_spawnp" };
		handle_ = ::_spawnvpe(_P_NOWAIT, path, argv, envp);
		if (handle_ == -1) errg.throw_error(location);
#endif
	}

#ifndef _WIN32
	pid_t pid() const { return pid_; }
#else
	intptr_t handle() const { return handle_; }
#endif

	int wait(source_location location = source_location::current())
	{
		int status = -1;
#ifndef _WIN32
		if (pid_ <= 0) return -1;
		errno_guard errg{ "waitpid" };
		pid_t res = ::waitpid(pid_, &status, 0);
		if (res < 0) errg.throw_error(location);
#else
		if (handle_ == -1) return -1;
		errno_guard errg{ "_cwait" };
		pid_t res = ::_cwait(&status, handle_, _WAIT_CHILD);
		if (res == -1) errg.throw_error(location);
#endif
		return status;
	}

	void wait_exit(std::function<bool(int)> pred = inf::status_equals<0>,
				   source_location location = source_location::current())
	{
		int status = wait();
#ifndef _WIN32
		if (!WIFEXITED(status)) throw status_exception(status, location);
		status = WEXITSTATUS(status);
#endif
		if (!pred(status)) throw status_exception(status, location);
	}

#ifndef _WIN32
	int peek(source_location location = source_location::current())
	{
		if (pid_ <= 0) return -1;
		siginfo_t status;
		status.si_pid = 0;
		errno_guard errg{ "waitid" };
		int res = ::waitid(P_PID, static_cast<id_t>(pid_), &status, WEXITED | WNOHANG | WNOWAIT);
		if (res < 0) errg.throw_error(location);
		return status.si_pid;
	}
#endif

private:
#ifndef _WIN32
	pid_t pid_;
#else
	intptr_t handle_;
#endif
};

} // namespace inf
