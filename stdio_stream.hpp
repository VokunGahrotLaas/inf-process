#pragma once

#include <version>

#if defined(__GLIBCXX__) || defined(__GLIBCPP__)
#	define INF_LIBSTDCPP
#elif defined(_LIBCPP_VERSION)
#	define INF_LIBCPP
#elif defined(_MSC_VER)
#	define INF_LIBMSVCCPP
#endif

#ifndef INF_LIBSTDCPP
#	error "sorry not for now, please compile with GNU's libstdc++"
#endif

#include <ext/stdio_filebuf.h>
#include <ext/stdio_sync_filebuf.h>
#include <iostream>
#include <memory>

namespace inf
{

template <typename CharT, typename Traits = std::char_traits<CharT>>
using stdio_filebuf = __gnu_cxx::stdio_sync_filebuf<CharT, Traits>;

template <typename CharT, typename Traits = std::char_traits<CharT>>
class stdio_iostream : public std::basic_iostream<CharT, Traits>
{
public:
	using super_type = std::basic_iostream<CharT, Traits>;

	explicit stdio_iostream(FILE* file)
		: super_type(&filebuf_)
		, filebuf_{ file }
	{}

	explicit stdio_iostream(int fd)
		: stdio_iostream{ fd >= 0 ? ::fdopen(fd, "rw") : nullptr }
	{}

	stdio_iostream(stdio_iostream const&) = delete;
	stdio_iostream& operator=(stdio_iostream const&) = delete;

	stdio_iostream(stdio_iostream&& other)
		: super_type(std::move(other))
		, filebuf_(std::move(other.filebuf_))
	{
		super_type::rdbuf(&filebuf_);
	}

	stdio_iostream& operator=(stdio_iostream&& other)
	{
		if (filebuf_.file() != nullptr) std::fclose(filebuf_.file());
		filebuf_ = std::move(other.filebuf_);
		super_type::operator=(std::move(other));
	}

	~stdio_iostream() = default;

	stdio_iostream& operator=(FILE* new_file)
	{
		if (new_file == file()) return *this;
		if (filebuf_.file() != nullptr) std::fclose(filebuf_.file());
		filebuf_ = stdio_filebuf<CharT, Traits>(new_file);
		return *this;
	}

	FILE* file() const { return filebuf_ ? filebuf_->file() : nullptr; }

	int fd() const { return file() != nullptr ? ::fileno(file()) : -1; }

private:
	std::unique_ptr<stdio_filebuf<CharT, Traits>, decltype([](stdio_filebuf<CharT, Traits>* streambuf) {
						if (streambuf == nullptr) return;
						if (streambuf->file() != nullptr) std::fclose(streambuf->file());
						delete streambuf;
					})>
		filebuf_;
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
class stdio_istream : public std::basic_istream<CharT, Traits>
{
public:
	using super_type = std::basic_istream<CharT, Traits>;

	explicit stdio_istream(FILE* file)
		: super_type(&filebuf_)
		, filebuf_{ file }
	{}

	explicit stdio_istream(int fd)
		: stdio_istream{ fd >= 0 ? ::fdopen(fd, "r") : nullptr }
	{}

	stdio_istream(stdio_istream const&) = delete;
	stdio_istream& operator=(stdio_istream const&) = delete;

	stdio_istream(stdio_istream&& other)
		: super_type(std::move(other))
		, filebuf_(std::move(other.filebuf_))
	{
		super_type::rdbuf(&filebuf_);
	}

	stdio_istream& operator=(stdio_istream&& other)
	{
		if (filebuf_.file() != nullptr) std::fclose(filebuf_.file());
		filebuf_ = std::move(other.filebuf_);
		super_type::operator=(std::move(other));
	}

	~stdio_istream() override = default;

	stdio_istream& operator=(FILE* new_file)
	{
		if (new_file == file()) return *this;
		if (filebuf_.file() != nullptr) std::fclose(filebuf_.file());
		filebuf_ = stdio_filebuf<CharT, Traits>(new_file);
		return *this;
	}

	bool is_open() { return filebuf_.is_open(); }

	FILE* file() { return filebuf_.file(); }

	int fd() { return file() != nullptr ? ::fileno(file()) : -1; }

private:
	stdio_filebuf<CharT, Traits> filebuf_;
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
class stdio_ostream : public std::basic_ostream<CharT, Traits>
{
public:
	using super_type = std::basic_ostream<CharT, Traits>;

	explicit stdio_ostream(FILE* file)
		: super_type(&filebuf_)
		, filebuf_{ file }
	{}

	explicit stdio_ostream(int fd)
		: stdio_ostream{ fd >= 0 ? ::fdopen(fd, "w") : nullptr }
	{}

	stdio_ostream(stdio_ostream const&) = delete;
	stdio_ostream& operator=(stdio_ostream const&) = delete;

	stdio_ostream(stdio_ostream&& other)
		: super_type(std::move(other))
		, filebuf_(std::move(other.filebuf_))
	{
		super_type::rdbuf(&filebuf_);
	}

	stdio_ostream& operator=(stdio_ostream&& other)
	{
		if (filebuf_.file() != nullptr) std::fclose(filebuf_.file());
		filebuf_ = std::move(other.filebuf_);
		super_type::operator=(std::move(other));
	}

	~stdio_ostream() override = default;

	stdio_ostream& operator=(FILE* new_file)
	{
		if (new_file == file()) return *this;
		if (filebuf_.file() != nullptr) std::fclose(filebuf_.file());
		filebuf_ = stdio_filebuf<CharT, Traits>(new_file);
		return *this;
	}

	FILE* file() { return filebuf_.file(); }

	int fd() { return file() != nullptr ? ::fileno(file()) : -1; }

private:
	stdio_filebuf<CharT, Traits> filebuf_;
};

} // namespace inf
