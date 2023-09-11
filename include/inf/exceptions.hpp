#pragma once

// STL
#include <exception>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>

namespace inf
{

class errno_error : public std::exception
{
public:
	errno_error(std::string_view func, int errno_nb,
				std::source_location location = std::source_location::current()) noexcept
		: what_{}
		, func_{ func }
		, errno_nb_(errno_nb)
	{
		std::stringstream ss;
		ss << location.file_name() << ':' << location.line() << ':' << location.column() << ':'
		   << location.function_name() << ": " << func_ << "() failed with errno " << errno_nb_;
		what_ = std::move(ss).str();
	}

	~errno_error() override = default;

	char const* what() const noexcept override { return what_.c_str(); }

	std::string const& func() { return func_; }

	int errno_nb() { return errno_nb_; }

private:
	std::string what_;
	std::string func_;
	int errno_nb_;
};

} // namespace inf
