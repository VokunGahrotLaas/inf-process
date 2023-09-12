#pragma once

#ifndef _WIN32

// Unix
#	include <sys/wait.h>
#	include <unistd.h>
// inf
#	include <inf/with_errno.hpp>

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
		if (with_errno werr{ "fork" })
		{
			pid_ = ::fork();
			if (pid_ < 0) werr.throw_error(location);
		}
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
		if (with_errno werr{ "waitpid" })
		{
			pid_t res = ::waitpid(pid_, &status, 0);
			if (res < 0) werr.throw_error(location);
		}
		return status;
	}

private:
	pid_t pid_;
};

} // namespace inf

#endif
