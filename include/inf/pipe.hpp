#pragma once

// for ::pipe
#ifdef _WIN32
#	include <fcntl.h>
#	include <io.h>
#else
#	include <unistd.h>
#endif
// inf
#include <inf/stdio_stream.hpp>
#include <inf/with_errno.hpp>

namespace inf
{

template <typename CharT, typename Traits = std::char_traits<CharT>>
class basic_pipe;

using pipe = basic_pipe<char>;
using wpipe = basic_pipe<wchar_t>;

template <typename CharT, typename Traits>
class basic_pipe
{
public:
	using istream_type = basic_stdio_istream<CharT, Traits>;
	using ostream_type = basic_stdio_ostream<CharT, Traits>;

	basic_pipe()
		: read{ nullptr }
		, write{ nullptr }
	{}

	explicit basic_pipe(int read, int write)
		: read{ istream_type::from_fd(read) }
		, write{ ostream_type::from_fd(write) }
	{}

	basic_pipe(basic_pipe const&) = delete;
	basic_pipe& operator=(basic_pipe const&) = delete;

	basic_pipe(basic_pipe&& other)
		: read{ std::move(other.read) }
		, write{ std::move(other.write) }
	{
		if (read.tie() == &other.write) read.tie(&write);
	}

	basic_pipe& operator=(basic_pipe&& other)
	{
		if (this == &other) return *this;
		read = std::move(other.read);
		write = std::move(other.write);
		if (read.tie() == &other.write) read.tie(&write);
		return *this;
	}

	~basic_pipe() = default;

	istream_type read;
	ostream_type write;
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline basic_pipe<CharT, Traits> make_basic_pipe([[maybe_unused]] unsigned size = 1'024,
												 std::source_location location = std::source_location::current())
{
	int fds[2] = { -1, -1 };
	if (with_errno werr{ "pipe" })
	{
#ifdef _WIN32
		bool success = ::_pipe(fds, size, _O_BINARY) >= 0;
#else
		bool success = ::pipe(fds) >= 0;
#endif
		if (!success) werr.throw_error(location);
	}
	return basic_pipe<CharT, Traits>(fds[0], fds[1]);
}

inline pipe make_pipe() { return make_basic_pipe<char>(); }

inline wpipe make_wpipe() { return make_basic_pipe<wchar_t>(); }

#if !defined(INF_HEADER_ONLY) && !defined(INF_STATIC_PIPE)
extern template class basic_pipe<char>;
extern template class basic_pipe<wchar_t>;
#endif

} // namespace inf
