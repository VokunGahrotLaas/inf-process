#pragma once

#include <cstdlib>
#include <iostream>
#include <sstream>
// inf
#include <inf/stdiobuf.hpp>

#ifndef _MSC_VER
#	define INF_GNU_PURE [[gnu::pure]]
#else
#	define INF_GNU_PURE
#endif

#ifndef _WIN32
#	define INF_FDOPEN ::fdopen
#else
#	define INF_FDOPEN ::_fdopen
#endif

namespace inf
{

enum class OpenMode : int
{
	MODE_NONE = 0,
	MODE_APP = 1 << 0,
	MODE_BIN = 1 << 1,
	MODE_IN = 1 << 2,
	MODE_OUT = 1 << 3,
};

INF_GNU_PURE
constexpr OpenMode operator|(OpenMode lhs, OpenMode rhs)
{
	return static_cast<OpenMode>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

constexpr OpenMode& operator|=(OpenMode& lhs, OpenMode rhs) { return lhs = lhs | rhs; }

INF_GNU_PURE
constexpr int open_mode_int(std::ios_base::openmode mode)
{
	OpenMode imode = OpenMode::MODE_NONE;
	if (mode & std::ios_base::app) imode |= OpenMode::MODE_APP;
	if (mode & std::ios_base::binary) imode |= OpenMode::MODE_BIN;
	if (mode & std::ios_base::in) imode |= OpenMode::MODE_IN;
	if (mode & std::ios_base::out) imode |= OpenMode::MODE_OUT;
	return static_cast<int>(imode);
}

INF_GNU_PURE
constexpr char const* open_mode_str(std::ios_base::openmode mode)
{
	constexpr char const* open_mode_strs[] = {
		"", "a", "b", "ab", "r", "a+", "rb", "a+b", "w", "a", "wb", "ab", "w+", "a+", "w+b", "a+b",
	};
	return open_mode_strs[open_mode_int(mode)];
}

template <typename CharT, typename Traits, template <typename CharT2, typename Traits2> class Stream,
		  std::ios_base::openmode DefaultMode>
class basic_stdio_stream : public Stream<CharT, Traits>
{
public:
	using super_type = Stream<CharT, Traits>;
	using buf_type = basic_stdiobuf<CharT, Traits>;

	explicit basic_stdio_stream(std::FILE* file)
		: super_type(&buf_)
		, buf_{ file }
	{}

	explicit basic_stdio_stream(int fd, std::ios_base::openmode mode = DefaultMode)
		: basic_stdio_stream{ fd >= 0 ? INF_FDOPEN(fd, open_mode_str(mode)) : nullptr }
	{}

	basic_stdio_stream(basic_stdio_stream const&) = delete;

	basic_stdio_stream(basic_stdio_stream&& other)
		: super_type(std::move(other))
		, buf_(std::move(other.buf_))
	{
		super_type::rdbuf(&buf_);
	}

	~basic_stdio_stream() = default;

	basic_stdio_stream& operator=(basic_stdio_stream const&) = delete;

	basic_stdio_stream& operator=(basic_stdio_stream&& other)
	{
		if (buf_.file() != nullptr) std::fclose(buf_.file());
		buf_ = std::move(other.buf_);
		super_type::operator=(std::move(other));
		super_type::rdbuf(&buf_);
		return *this;
	}

	basic_stdio_stream& operator=(std::FILE* new_file)
	{
		if (new_file == file()) return *this;
		buf_ = buf_type(new_file);
		return *this;
	}

	bool is_open() const { return buf_.is_open(); }

	std::FILE* file() { return buf_.file(); }

	int fd() { return file() != nullptr ? ::fileno(file()) : -1; }

	buf_type& buf() { return buf_; }

private:
	buf_type buf_;
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
using basic_stdio_iostream =
	basic_stdio_stream<CharT, Traits, std::basic_iostream, std::ios_base::in | std::ios_base::out>;

template <typename CharT, typename Traits = std::char_traits<CharT>>
using basic_stdio_istream = basic_stdio_stream<CharT, Traits, std::basic_istream, std::ios_base::in>;

template <typename CharT, typename Traits = std::char_traits<CharT>>
using basic_stdio_ostream = basic_stdio_stream<CharT, Traits, std::basic_ostream, std::ios_base::out>;

using stdio_iostream = basic_stdio_iostream<char>;
using wstdio_iostream = basic_stdio_iostream<wchar_t>;

using stdio_istream = basic_stdio_istream<char>;
using wstdio_istream = basic_stdio_istream<wchar_t>;

using stdio_ostream = basic_stdio_ostream<char>;
using wstdio_ostream = basic_stdio_ostream<wchar_t>;

#ifdef INF_EXTERN_TEMPLATE
extern template class basic_stdio_stream<char, std::char_traits<char>, std::basic_iostream,
										 std::ios_base::in | std::ios_base::out>;
extern template class basic_stdio_stream<wchar_t, std::char_traits<wchar_t>, std::basic_iostream,
										 std::ios_base::in | std::ios_base::out>;

extern template class basic_stdio_stream<char, std::char_traits<char>, std::basic_istream, std::ios_base::in>;
extern template class basic_stdio_stream<wchar_t, std::char_traits<wchar_t>, std::basic_istream, std::ios_base::in>;

extern template class basic_stdio_stream<char, std::char_traits<char>, std::basic_ostream, std::ios_base::out>;
extern template class basic_stdio_stream<wchar_t, std::char_traits<wchar_t>, std::basic_ostream, std::ios_base::out>;
#endif

} // namespace inf
