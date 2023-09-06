#pragma once

// errno for ::pipe
#include <cerrno>
// for ::pipe
#include <unistd.h>
// inf
#include <inf/exceptions.hpp>
#include <inf/stdio_stream.hpp>
#include <utility>

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
	basic_pipe()
		: read_{ nullptr }
		, write_{ nullptr }
	{}

	basic_pipe(basic_pipe const&) = delete;
	basic_pipe& operator=(basic_pipe const&) = delete;

	basic_pipe(basic_pipe&& other) = default;

	basic_pipe& operator=(basic_pipe&& other)
	{
		if (this == &other) return *this;
		close();
		read_ = std::exchange(other.read_, nullptr);
		write_ = std::exchange(other.write_, nullptr);
		if (read_.tie() == &other.write_) read_.tie(&write_);
		return *this;
	}

	~basic_pipe() { close(); }

	void close()
	{
		read_close();
		write_close();
	}

	bool read_close()
	{
		std::FILE* old_file = read_.file();
		read_ = nullptr;
		return old_file != nullptr;
	}

	bool write_close()
	{
		if (read_.tie() == &write_) read_.tie(nullptr);
		std::FILE* old_file = write_.file();
		write_ = nullptr;
		return old_file != nullptr;
	}

	stdio_istream& read() { return read_; }

	stdio_ostream& write() { return write_; }

	template <typename CharT2, typename Traits2>
	friend inline basic_pipe<CharT2, Traits2> make_basic_pipe();
	friend inline pipe make_pipe();
	friend inline wpipe make_wpipe();

private:
	basic_pipe(int read, int write)
		: read_{ read }
		, write_{ write }
	{
		read_.tie(&write_);
	}

	stdio_istream read_;
	stdio_ostream write_;
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline basic_pipe<CharT, Traits> make_basic_pipe()
{
	int raw_pipe[2] = { -1, -1 };
	int old_errno = errno;
	bool success = ::pipe(raw_pipe) >= 0;
	int failure_errno = errno;
	errno = old_errno;
	if (!success) throw errno_error("pipe", failure_errno);
	return basic_pipe<CharT, Traits>(raw_pipe[0], raw_pipe[1]);
}

inline pipe make_pipe() { return make_basic_pipe<char>(); }

inline wpipe make_wpipe() { return make_basic_pipe<wchar_t>(); }

#ifdef INF_EXTERN_TEMPLATE
extern template class basic_pipe<char>;
extern template class basic_pipe<wchar_t>;
#endif

} // namespace inf
