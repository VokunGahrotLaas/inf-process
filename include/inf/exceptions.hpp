#pragma once

// STL
#include <exception>
#include <sstream>
#include <string>
#include <string_view>
// inf
#include <inf/source_location.hpp>

namespace inf
{

namespace
{

template <typename... Args>
std::string string_of_stream(Args&&... args)
{
	std::stringstream ss;
	(ss << ... << std::forward<Args>(args));
	return std::move(ss).str();
}

} // namespace

class exception : public std::exception
{
public:
	using super_type = std::exception;

	exception(std::string_view what, inf::source_location location = inf::source_location::current()) noexcept
		: what_{ string_of_stream(location.file_name(), ':', location.line(), ':', location.column(), ':', what) }
	{}

	~exception() override = default;

	char const* what() const noexcept override { return what_.c_str(); }

private:
	std::string what_;
};

class errno_exception : public exception
{
public:
	using super_type = exception;

	errno_exception(std::string_view func, int errno_nb,
					inf::source_location location = inf::source_location::current()) noexcept
		: super_type{ string_of_stream("inf::errno_exception: ", func, "() failed with errno ", errno_nb_), location }
		, func_{ func }
		, errno_nb_(errno_nb)
	{}

	~errno_exception() override = default;

	std::string const& func() const noexcept { return func_; }

	int errno_nb() const noexcept { return errno_nb_; }

private:
	std::string func_;
	int errno_nb_;
};

class status_exception : public exception
{
public:
	using super_type = exception;

	status_exception(int status, inf::source_location location = inf::source_location::current()) noexcept
		: super_type{ string_of_stream("inf::status_exception: "), location }
		, status_{ status }
	{}

	~status_exception() override = default;

	int status() const noexcept { return status_; }

private:
	int status_;
};

} // namespace inf
