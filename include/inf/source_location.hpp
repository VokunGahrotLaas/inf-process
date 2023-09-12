#ifdef __cpp_lib_source_location

#	include <source_location>

namespace inf
{

using source_location = std::source_location;

} // namespace inf

#else

#	include <cstdint>

#	ifdef __GNUC__
#		define INF_FUNCTION __PRETTY_FUNCTION__
#	else
#		define INF_FUNCTION __func__
#	endif

namespace inf
{

struct source_location
{
	static constexpr source_location current(char const* file = __FILE__, char const* func = INF_FUNCTION,
											 int line = __LINE__, int col = 0) noexcept
	{
		source_location loc;
		loc.file_ = file;
		loc.func_ = func;
		loc.line_ = static_cast<decltype(loc.line_)>(line);
		loc.col_ = static_cast<decltype(loc.col_)>(col);
		return loc;
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
	char const* file_;
	char const* func_;
	uint_least32_t line_;
	uint_least32_t col_;
};

} // namespace inf

#endif
