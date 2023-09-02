#pragma once

#include <version>

#if defined(__GLIBCXX__) || defined(__GLIBCPP__)
#	define INF_LIBSTDCPP
#elif defined(_LIBCPP_VERSION)
#	define INF_LIBCPP
#endif

#ifndef _MSC_VER
#	define INF_FDOPEN ::fdopen
#else
#	define INF_FDOPEN ::_fdopen
#endif

#ifdef INF_LIBSTDCPP
#	include <ext/stdio_filebuf.h>
#	include <ext/stdio_sync_filebuf.h>
#else
#	include <fstream>
#endif

#include <iostream>
#include <memory>

namespace inf
{

#ifdef INF_LIBSTDCPP
template <typename CharT, typename Traits = std::char_traits<CharT>>
using stdio_filebuf = __gnu_cxx::stdio_sync_filebuf<CharT, Traits>;

template <typename CharT, typename Traits = std::char_traits<CharT>>
using fclose_stdio_filebuf_t = decltype([](stdio_filebuf<CharT, Traits>* streambuf) {
	if (streambuf == nullptr) return;
	if (streambuf->file() != nullptr) std::fclose(streambuf->file());
	delete streambuf;
});

#else

using fclose_t = decltype([](FILE* file) {
	if (file != nullptr) std::fclose(file);
});

#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
class stdio_iostream
{
public:
	explicit stdio_iostream(FILE* file)
#ifdef INF_LIBSTDCPP
		: filebuf_{ new stdio_filebuf<CharT, Traits>(file) }
		, stream_{ new std::basic_iostream<CharT, Traits>(filebuf_.get()) }
#else
		: file_{ file }
		, stream_{ new std::basic_fstream<CharT, Traits>(file_.get()) }
#endif
	{}

	explicit stdio_iostream(int fd)
		: stdio_iostream{ fd >= 0 ? INF_FDOPEN(fd, "rw") : nullptr }
	{}

	stdio_iostream(stdio_iostream const&) = delete;
	stdio_iostream& operator=(stdio_iostream const&) = delete;

	stdio_iostream(stdio_iostream&&) = default;
	stdio_iostream& operator=(stdio_iostream&&) = default;

	~stdio_iostream() = default;

	stdio_iostream& operator=(FILE* new_file)
	{
		if (new_file == file()) return *this;
#ifdef INF_LIBSTDCPP
		filebuf_.reset(new stdio_filebuf<CharT, Traits>(new_file));
		stream_.reset(new std::basic_iostream<CharT, Traits>(filebuf_.get()));
#else
		file_.reset(new_file);
		stream_.reset(new std::basic_fstream<CharT, Traits>(file_.get()));
#endif
		return *this;
	}

	operator std::basic_iostream<CharT, Traits>&() { return stream(); }

	std::basic_iostream<CharT, Traits>& stream() { return *stream_; }

	FILE* file() const
	{
#ifdef INF_LIBSTDCPP
		return filebuf_ ? filebuf_->file() : nullptr;
#else
		return file_.get();
#endif
	}

	int fd() const { return file() != nullptr ? ::fileno(file()) : -1; }

private:
#ifdef INF_LIBSTDCPP
	std::unique_ptr<stdio_filebuf<CharT, Traits>, decltype([](stdio_filebuf<CharT, Traits>* streambuf) {
						if (streambuf == nullptr) return;
						if (streambuf->file() != nullptr) std::fclose(streambuf->file());
						delete streambuf;
					})>
		filebuf_;
#else
	std::unique_ptr<FILE, fclose_t> file_;
#endif
	std::unique_ptr<std::basic_iostream<CharT, Traits>> stream_;
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
class stdio_istream
{
public:
	explicit stdio_istream(FILE* file)
#ifdef INF_LIBSTDCPP
		: filebuf_{ new stdio_filebuf<CharT, Traits>(file) }
		, stream_{ new std::basic_istream<CharT, Traits>(filebuf_.get()) }
#else
		: file_{ file }
		, stream_{ new std::basic_ifstream<CharT, Traits>(file_.get()) }
#endif
	{}

	explicit stdio_istream(int fd)
		: stdio_istream{ fd >= 0 ? INF_FDOPEN(fd, "r") : nullptr }
	{}

	stdio_istream(stdio_istream const&) = delete;
	stdio_istream& operator=(stdio_istream const&) = delete;

	stdio_istream(stdio_istream&&) = default;
	stdio_istream& operator=(stdio_istream&&) = default;

	~stdio_istream() = default;

	stdio_istream& operator=(FILE* new_file)
	{
		if (new_file == file()) return *this;
#ifdef INF_LIBSTDCPP
		filebuf_.reset(new stdio_filebuf<CharT, Traits>(new_file));
		stream_.reset(new std::basic_istream<CharT, Traits>(filebuf_.get()));
#else
		file_.reset(new_file);
		stream_.reset(new std::basic_ifstream<CharT, Traits>(file_.get()));
#endif
		return *this;
	}

	operator std::basic_istream<CharT, Traits>&() { return stream(); }

	std::basic_istream<CharT, Traits>& stream() { return *stream_; }

	FILE* file() const
	{
#ifdef INF_LIBSTDCPP
		return filebuf_ ? filebuf_->file() : nullptr;
#else
		return file_.get();
#endif
	}

	int fd() const { return file() != nullptr ? ::fileno(file()) : -1; }

private:
#ifdef INF_LIBSTDCPP
	std::unique_ptr<stdio_filebuf<CharT, Traits>, decltype([](stdio_filebuf<CharT, Traits>* streambuf) {
						if (streambuf == nullptr) return;
						if (streambuf->file() != nullptr) std::fclose(streambuf->file());
						delete streambuf;
					})>
		filebuf_;
#else
	std::unique_ptr<FILE, fclose_t> file_;
#endif
	std::unique_ptr<std::basic_istream<CharT, Traits>> stream_;
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
class stdio_ostream
{
public:
	explicit stdio_ostream(FILE* file)
#ifdef INF_LIBSTDCPP
		: filebuf_{ new stdio_filebuf<CharT, Traits>(file) }
		, stream_{ new std::basic_ostream<CharT, Traits>(filebuf_.get()) }
#else
		: file_{ file }
		, stream_{ new std::basic_ofstream<CharT, Traits>(file_.get()) }
#endif
	{}

	explicit stdio_ostream(int fd)
		: stdio_ostream{ fd >= 0 ? INF_FDOPEN(fd, "w") : nullptr }
	{}

	stdio_ostream(stdio_ostream const&) = delete;
	stdio_ostream& operator=(stdio_ostream const&) = delete;

	stdio_ostream(stdio_ostream&&) = default;
	stdio_ostream& operator=(stdio_ostream&&) = default;

	~stdio_ostream() = default;

	stdio_ostream& operator=(FILE* new_file)
	{
		if (new_file == file()) return *this;
#ifdef INF_LIBSTDCPP
		filebuf_.reset(new stdio_filebuf<CharT, Traits>(new_file));
		stream_.reset(new std::basic_ostream<CharT, Traits>(filebuf_.get()));
#else
		file_.reset(new_file);
		stream_.reset(new std::basic_ofstream<CharT, Traits>(file_.get()));
#endif
		return *this;
	}

	operator std::basic_ostream<CharT, Traits>&() { return stream(); }

	std::basic_ostream<CharT, Traits>& stream() { return *stream_; }

	FILE* file() const
	{
#ifdef INF_LIBSTDCPP
		return filebuf_ ? filebuf_->file() : nullptr;
#else
		return file_.get();
#endif
	}

	int fd() const { return file() != nullptr ? ::fileno(file()) : -1; }

private:
#ifdef INF_LIBSTDCPP
	std::unique_ptr<stdio_filebuf<CharT, Traits>, decltype([](stdio_filebuf<CharT, Traits>* streambuf) {
						if (streambuf == nullptr) return;
						if (streambuf->file() != nullptr) std::fclose(streambuf->file());
						delete streambuf;
					})>
		filebuf_;
#else
	std::unique_ptr<FILE, fclose_t> file_;
#endif
	std::unique_ptr<std::basic_ostream<CharT, Traits>> stream_;
};

} // namespace inf
