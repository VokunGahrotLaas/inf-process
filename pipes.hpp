#pragma once

// STL
#include <expected>
#include <optional>
// platform specific
#ifdef _MSC_VER
#	include <fstream>
#	include <io.h>
#else // _MSC_VER
#	include <ext/stdio_filebuf.h>
#	include <ext/stdio_sync_filebuf.h>
#	include <iostream>
#	include <unistd.h>
#endif
// C std
#include <cerrno>
#include <cstdio>

namespace inf
{

using errno_t = std::remove_cvref_t<decltype(errno)>; // int
using opt_errno_t = std::optional<errno_t>;

class Pipe
{
public:
	Pipe()
		: read_{ nullptr }
		, write_{ nullptr }
#ifndef _MSC_VER
		, readbuf_(read_)
		, writebuf_(write_)
#endif
	{}

	Pipe(Pipe const&) = delete;
	Pipe& operator=(Pipe const&) = delete;

	Pipe(Pipe&& other)
		: read_{ std::move(other.read_) }
		, write_{ std::move(other.write_) }
#ifndef _MSC_VER
		, readbuf_{ std::move(other.readbuf_) }
		, writebuf_{ std::move(other.writebuf_) }
#endif
	{
		other.read_ = other.write_ = nullptr;
	}

	Pipe& operator=(Pipe&& other)
	{
		if (this == &other) return *this;
		close();
		read_ = std::move(other.read_);
		write_ = std::move(other.write_);
#ifndef _MSC_VER
		readbuf_ = std::move(other.readbuf_);
		writebuf_ = std::move(other.writebuf_);
#endif
		other.read_ = other.write_ = nullptr;
		return *this;
	}

	~Pipe() { close(); }

	void close()
	{
		read_close();
		write_close();
	}

	bool read_close()
	{
		FILE* old_file = read_;
		read_ = nullptr;
		return old_file != nullptr ? ::fclose(old_file) == 0 : false;
	}

	bool write_close()
	{
		FILE* old_file = write_;
		write_ = nullptr;
		return old_file != nullptr ? ::fclose(old_file) == 0 : false;
	}

	bool is_open() const { return read_is_open() && write_is_open(); }

	bool read_is_open() const { return read_ != nullptr; }

	bool write_is_open() const { return write_ != nullptr; }

#ifndef _MSC_VER
	std::istream read() { return std::istream(&readbuf_); }
	std::ostream write() { return std::ostream(&writebuf_); }
#else
	std::ifstream read() { return std::ifstream(read_); }
	std::ofstream write() { return std::ofstream(write_); }
#endif

	FILE* read_file() { return read_; }

	FILE* write_file() { return write_; }

	friend inline std::expected<Pipe, errno_t> make_pipe();

private:
	Pipe(int read, int write)
		: read_(::fdopen(read, "r"))
		, write_(::fdopen(write, "w"))
#ifndef _MSC_VER
		, readbuf_(read_)
		, writebuf_(write_)
#endif
	{}

	FILE* read_;
	FILE* write_;
#ifndef _MSC_VER
	__gnu_cxx::stdio_sync_filebuf<char> readbuf_;
	__gnu_cxx::stdio_sync_filebuf<char> writebuf_;
#endif
};

inline std::expected<Pipe, errno_t> make_pipe()
{
	int raw_pipe[2] = { -1, -1 };
	int old_errno = errno;
#ifndef _MSC_VER
	auto res = ::pipe(raw_pipe) < 0
#else
	auto res = ::_pipe(raw_pipe, 4'096, 0) < 0
#endif
		? std::unexpected(errno)
		: std::expected<Pipe, errno_t>(std::in_place, Pipe(raw_pipe[0], raw_pipe[1]));
	errno = old_errno;
	return res;
}

} // namespace inf
