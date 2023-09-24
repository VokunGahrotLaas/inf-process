#pragma once

#ifndef _WIN32

// Unix
#	include <sys/wait.h>
#	include <unistd.h>
// inf
#	include <inf/errno_guard.hpp>

namespace inf
{

class fork
{
public:
	enum class state_type
	{
		CHILD,
		PARENT
	};

	fork(inf::source_location location = inf::source_location::current())
		: pid_{ -1 }
	{
		errno_guard errg{ "fork" };
		pid_ = ::fork();
		if (pid_ < 0) errg.throw_error(location);
	}

	state_type state() const { return pid_ == 0 ? state_type::CHILD : state_type::PARENT; }

	operator state_type() const { return state(); }

	bool is_parent() const { return state() == state_type::PARENT; }

	bool is_child() const { return state() == state_type::CHILD; }

	pid_t pid() const { return pid_; }

	int wait(inf::source_location location = inf::source_location::current())
	{
		if (pid_ <= 0) return -1;
		int status = -1;
		errno_guard errg{ "waitpid" };
		pid_t res = ::waitpid(pid_, &status, 0);
		if (res < 0) errg.throw_error(location);
		return status;
	}

	bool peek(inf::source_location location = inf::source_location::current())
	{
		if (pid_ <= 0) return -1;
		siginfo_t status;
		status.si_pid = 0;
		errno_guard errg{ "waitid" };
		int res = ::waitid(P_PID, static_cast<id_t>(pid_), &status, WEXITED | WNOHANG | WNOWAIT);
		if (res < 0) errg.throw_error(location);
		return status.si_pid > 0;
	}

private:
	pid_t pid_;
};

} // namespace inf

#endif
