#pragma once

// STL
#include <cerrno>
#include <source_location>
#include <string>
// inf
#include <inf/exceptions.hpp>
#include <inf/with.hpp>

namespace inf
{

class with_errno : public with
{
public:
	with_errno(std::string func)
		: old_errno_(errno)
		, func_(func)
	{
		errno = 0;
	}

	~with_errno() { errno = old_errno_; }

	void throw_error(std::source_location location = std::source_location::current())
	{
		throw errno_error(func_, errno, location);
	}

private:
	int old_errno_;
	std::string func_;
};

} // namespace inf
