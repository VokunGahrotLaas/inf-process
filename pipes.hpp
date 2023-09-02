#pragma once

// STL
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
// platform specific
#ifdef _MSC_VER
#	include <io.h>
#else // _MSC_VER
#	include <unistd.h>
#endif
// C std
#include <cerrno>
#include <cstdio>
// inf
#include "stdio_stream.hpp"

namespace inf
{

template <typename CharT, typename Traits>
class pipe;

using errno_t = std::remove_cvref_t<decltype(errno)>; // int
using opt_errno_t = std::optional<errno_t>;

template <typename CharT, typename Traits = std::char_traits<CharT>>
class pipe
{
public:
	pipe()
		: read_{ nullptr }
		, write_{ nullptr }
	{}

	pipe(pipe const&) = delete;
	pipe& operator=(pipe const&) = delete;

	pipe(pipe&& other) = default;

	pipe& operator=(pipe&& other)
	{
		if (this == &other) return *this;
		close();
		read_ = std::move(other.read_);
		write_ = std::move(other.write_);
		other.read_ = other.write_ = nullptr;
		return *this;
	}

	~pipe() { close(); }

	void close()
	{
		read_close();
		write_close();
	}

	bool read_close()
	{
		FILE* old_file = read_.file();
		read_ = nullptr;
		return old_file != nullptr;
	}

	bool write_close()
	{
		FILE* old_file = write_.file();
		write_ = nullptr;
		return old_file != nullptr;
	}

	bool is_open() const { return read_is_open() && write_is_open(); }

	bool read_is_open() const { return read_.file() != nullptr; }

	bool write_is_open() const { return write_.file() != nullptr; }

	std::istream& read() { return read_.stream(); }
	std::ostream& write() { return write_.stream(); }

	FILE* read_file() { return read_.file(); }

	FILE* write_file() { return write_.file(); }

	template <typename CharT2, typename Traits2>
	friend inline pipe<CharT2, Traits2> make_pipe();

private:
	pipe(int read, int write)
		: read_{ read }
		, write_{ write }
	{}

	stdio_istream<char> read_;
	stdio_ostream<char> write_;
};

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

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline pipe<CharT, Traits> make_pipe()
{
	int raw_pipe[2] = { -1, -1 };
	int old_errno = errno;
#ifndef _MSC_VER
	bool success = ::pipe(raw_pipe) >= 0;
#else
	bool success = ::_pipe(raw_pipe, 4'096, 0) >= 0;
#endif
	int failure_errno = errno;
	errno = old_errno;
	if (!success) throw errno_error("pipe", failure_errno);
	return pipe<CharT, Traits>(raw_pipe[0], raw_pipe[1]);
}

} // namespace inf
