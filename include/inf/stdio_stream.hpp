#pragma once

// STL
#include <iostream>
#include <utility>
// Windows
#ifdef _WIN32
#	include <windows.h>
#endif
// inf
#include <inf/errno_guard.hpp>
#include <inf/ioutils.hpp>
#include <inf/stdiobuf.hpp>

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

#ifdef _WIN32
INF_GNU_PURE
constexpr int open_mode_winflags(std::ios_base::openmode mode, int o_text)
{
	int flags = 0;
	if (mode & std::ios_base::app) flags |= _O_APPEND;
	if (!(mode & std::ios_base::binary)) flags |= o_text;
	if ((mode & std::ios_base::in) && !(mode & std::ios_base::out)) flags |= _O_RDONLY;
	return flags;
}
#endif

INF_GNU_PURE
constexpr char const* open_mode_str(std::ios_base::openmode mode)
{
	constexpr char const* open_mode_strs[] = {
		"", "a", "b", "ab", "r", "a+", "rb", "a+b", "w", "a", "wb", "ab", "w+", "a+", "w+b", "a+b",
	};
	return open_mode_strs[open_mode_int(mode)];
}

INF_GNU_PURE
constexpr wchar_t const* wopen_mode_str(std::ios_base::openmode mode)
{
	constexpr wchar_t const* open_mode_strs[] = {
		L"", L"a", L"b", L"ab", L"r", L"a+", L"rb", L"a+b", L"w", L"a", L"wb", L"ab", L"w+", L"a+", L"w+b", L"a+b",
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

	static basic_stdio_stream from_fd(int fd, std::ios_base::openmode mode = DefaultMode)
	{
#ifdef _WIN32
		return basic_stdio_stream{ fd >= 0 ? (std::is_same_v<CharT, char> ? INF_FDOPEN(fd, open_mode_str(mode))

																		  : INF_WFDOPEN(fd, wopen_mode_str(mode)))
										   : nullptr };
#else
		return basic_stdio_stream{ fd >= 0 ? INF_FDOPEN(fd, open_mode_str(mode)) : nullptr };

#endif
	}

#ifdef _WIN32
	static basic_stdio_stream from_handle(HANDLE handle, std::ios_base::openmode mode = DefaultMode,
										  inf::source_location location = inf::source_location::current())
	{
		return basic_stdio_stream::from_fd(winhandle_to_fd(
			handle, open_mode_winflags(mode, (std::is_same_v<CharT, char> ? _O_TEXT : _O_WTEXT)), location));
	}
#endif

	basic_stdio_stream(basic_stdio_stream const&) = delete;

	basic_stdio_stream(basic_stdio_stream&& other)
		: super_type(std::move(other))
		, buf_(std::exchange(other.buf_, buf_type(nullptr)))
	{
		super_type::rdbuf(&buf_);
	}

	~basic_stdio_stream() = default;

	basic_stdio_stream& operator=(basic_stdio_stream const&) = delete;

	basic_stdio_stream& operator=(basic_stdio_stream&& other)
	{
		if (buf_.file() != nullptr) std::fclose(buf_.file());
		buf_ = std::exchange(other.buf_, buf_type(nullptr));
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

#ifndef _WIN32
	int native_handle([[maybe_unused]] inf::source_location location = inf::source_location::current()) { return fd(); }
#else
	HANDLE native_handle(inf::source_location location = inf::source_location::current())
	{
		return fd_to_winhandle(fd(), location);
	}
#endif

	buf_type& buf() { return buf_; }

	void close() { buf_.close(); }

	basic_stdio_stream dup(std::ios_base::openmode mode = DefaultMode,
						   inf::source_location location = inf::source_location::current()) const
	{
		int new_fd = -1;
		errno_guard errg{ "dup" };
		new_fd = INF_DUP(private_fd());
		if (new_fd < 0) errg.throw_error(location);
		return basic_stdio_stream::from_fd(new_fd, mode);
	}

	void dup(basic_stdio_stream& other, inf::source_location location = inf::source_location::current()) const
	{
		errno_guard errg{ "dup2" };
		int res = INF_DUP2(private_fd(), other.fd());
		if (res < 0) errg.throw_error(location);
	}

	basic_stdio_stream safe_dup(std::ios_base::openmode mode = DefaultMode,
								inf::source_location location = inf::source_location::current())
	{
		basic_stdio_stream dupped = dup(mode, location);
		close();
		return dupped;
	}

	basic_stdio_stream safe_dup(basic_stdio_stream& other, std::ios_base::openmode mode = DefaultMode,
								inf::source_location location = inf::source_location::current())
	{
		basic_stdio_stream backup = other.dup(mode, location);
		dup(other, location);
		close();
		return backup;
	}

	void dup_back(basic_stdio_stream& other, inf::source_location location = inf::source_location::current())
	{
		dup(other, location);
		close();
	}

private:
	std::FILE* private_file() const { return buf_.private_file(); }

	int private_fd() const { return private_file() != nullptr ? ::fileno(private_file()) : -1; }

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

#if !defined(INF_HEADER_ONLY) && !defined(INF_STATIC_STDIO_STREAM)
extern template class basic_stdio_stream<char, std::char_traits<char>, std::basic_iostream,
										 std::ios_base::in | std::ios_base::out>;
extern template class basic_stdio_stream<wchar_t, std::char_traits<wchar_t>, std::basic_iostream,
										 std::ios_base::in | std::ios_base::out>;

extern template class basic_stdio_stream<char, std::char_traits<char>, std::basic_istream, std::ios_base::in>;
extern template class basic_stdio_stream<wchar_t, std::char_traits<wchar_t>, std::basic_istream, std::ios_base::in>;

extern template class basic_stdio_stream<char, std::char_traits<char>, std::basic_ostream, std::ios_base::out>;
extern template class basic_stdio_stream<wchar_t, std::char_traits<wchar_t>, std::basic_ostream, std::ios_base::out>;

extern stdio_ostream cout;
extern wstdio_ostream wcout;

extern stdio_ostream cerr;
extern wstdio_ostream wcerr;

extern stdio_istream cin;
extern wstdio_istream wcin;
#else
stdio_ostream cout{ stdout };
wstdio_ostream wcout{ stdout };

stdio_ostream cerr{ stderr };
wstdio_ostream wcerr{ stderr };

stdio_istream cin{ stdin };
wstdio_istream wcin{ stdin };
#endif

} // namespace inf
