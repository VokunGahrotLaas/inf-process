#pragma once

#ifndef _WIN32

// STL
#	include <functional>
// Unix
#	include <sys/wait.h>
#	include <unistd.h>
// inf
#	include <inf/errno_guard.hpp>
#	include <inf/exceptions.hpp>

namespace inf
{

template <int expected>
std::function<bool(int)> status_equals = [](int status) -> bool { return status == expected; };

class fork
{
public:
	enum class state_type
	{
		CHILD,
		PARENT
	};

	fork(source_location location = source_location::current())
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

	int wait(source_location location = source_location::current())
	{
		if (pid_ <= 0) return -1;
		int status = -1;
		errno_guard errg{ "waitpid" };
		pid_t res = ::waitpid(pid_, &status, 0);
		if (res < 0) errg.throw_error(location);
		return status;
	}

	void wait_exit(std::function<bool(int)> pred = inf::status_equals<0>,
				   source_location location = source_location::current())
	{
		int status = wait();
		if (!WIFEXITED(status)) throw status_exception(status, location);
		status = WEXITSTATUS(status);
		if (!pred(status)) throw status_exception(status, location);
	}

	bool peek(source_location location = source_location::current())
	{
		if (pid_ <= 0) return -1;
		siginfo_t status;
		status.si_pid = 0;
		errno_guard errg{ "waitid" };
		int res = ::waitid(P_PID, static_cast<id_t>(pid_), &status, WEXITED | WNOHANG | WNOWAIT);
		if (res < 0) errg.throw_error(location);
		return status.si_pid;
	}

private:
	pid_t pid_;
};

} // namespace inf

#endif
