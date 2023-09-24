#pragma once

// STL
#include <cerrno>
#include <string>
// inf
#include <inf/exceptions.hpp>

namespace inf
{

class errno_guard
{
public:
	errno_guard(std::string func)
		: old_errno_(errno)
		, func_(func)
	{
		errno = 0;
	}

	~errno_guard() { errno = old_errno_; }

	void throw_error(inf::source_location location = inf::source_location::current())
	{
		throw errno_exception(func_, errno, location);
	}

private:
	int old_errno_;
	std::string func_;
};

} // namespace inf
