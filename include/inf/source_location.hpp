#pragma once

#include <version>

#ifdef __cpp_lib_source_location

#	include <source_location>

namespace inf
{

using source_location = std::source_location;

} // namespace inf

#else

#	include <cstdint>

namespace inf
{

// https://github.com/gcc-mirror/gcc/blob/1f973c295b699011acd7d5fcb04bd2bf887da16d/libstdc%2B%2B-v3/include/experimental/source_location
struct source_location
{
	static constexpr source_location current(char const* file = __builtin_FILE(),
											 char const* func = __builtin_FUNCTION(), int line = __builtin_LINE(),
											 int col = 0) noexcept
	{
		return source_location{ file, func, static_cast<uint_least32_t>(line), static_cast<uint_least32_t>(col) };
	}

	constexpr source_location() noexcept
		: file_("unknown")
		, func_(file_)
		, line_(0)
		, col_(0)
	{}

	constexpr uint_least32_t line() const noexcept { return line_; }
	constexpr uint_least32_t column() const noexcept { return col_; }
	constexpr char const* file_name() const noexcept { return file_; }
	constexpr char const* function_name() const noexcept { return func_; }

private:
	constexpr explicit source_location(char const* file, char const* func, uint_least32_t line,
									   uint_least32_t col) noexcept
		: file_(file)
		, func_(func)
		, line_(line)
		, col_(col)
	{}

	char const* file_;
	char const* func_;
	uint_least32_t line_;
	uint_least32_t col_;
};

} // namespace inf

#endif
