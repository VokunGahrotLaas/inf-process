#pragma once

// STL
#include <iostream>
#include <mutex>

namespace inf
{

template <typename Stream>
class stream_lock
{
public:
	stream_lock(Stream& stream, std::mutex& mutex)
		: stream_{ stream }
		, lock_{ mutex }
	{}

	stream_lock(stream_lock const&) = delete;
	stream_lock(stream_lock&&) = default;

	stream_lock& operator=(stream_lock const&) = delete;
	stream_lock& operator=(stream_lock&&) = default;

	Stream& stream() { return stream_; }

private:
	Stream& stream_;
	std::unique_lock<std::mutex> lock_;
};

template <typename Stream, typename CharT = typename Stream::char_type, typename Traits = typename Stream::traits_type>
stream_lock<Stream> operator<<(stream_lock<Stream>&& stream,
							   std::basic_ostream<CharT, Traits>& (*func)(std::basic_ostream<CharT, Traits>&))
{
	stream_lock<Stream> s{ std::move(stream) };
	s.stream() << func;
	return s;
}

template <typename Stream, typename T>
stream_lock<Stream> operator<<(stream_lock<Stream>&& stream, T&& value)
{
	stream_lock<Stream> s{ std::move(stream) };
	s.stream() << std::forward<T>(value);
	return s;
}

template <typename Stream, typename T>
stream_lock<Stream> operator>>(stream_lock<Stream>&& stream, T&& value)
{
	stream_lock<Stream> s{ std::move(stream) };
	s.stream() >> std::forward<T>(value);
	return s;
}

class stream_lockable_base
{
	virtual std::mutex& mutex() = 0;
};

template <typename Stream>
class stream_lockable : public stream_lockable_base
{
public:
	std::mutex& mutex() override { return mutex_; }

	stream_lock<Stream> lock() { return stream_lock<Stream>{ *dynamic_cast<Stream*>(this), mutex() }; }

private:
	std::mutex mutex_;
};

} // namespace inf
