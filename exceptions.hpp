#pragma once

#include <exception>
#include <sstream>
#include <string>
#include <string_view>

namespace inf
{

class errno_error : public std::exception
{
public:
	errno_error(std::string_view func, int errno_) noexcept
		: what_{}
	{
		std::stringstream ss;
		ss << func << "() failed with errno " << errno_;
		what_ = std::move(ss).str();
	}

	~errno_error() override = default;

	char const* what() const noexcept override { return what_.c_str(); }

private:
	std::string what_;
};

} // namespace inf
