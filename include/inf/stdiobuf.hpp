#pragma once

// STL
#include <cstdio>
#include <streambuf>
#include <utility>

namespace inf
{

template <typename CharT, typename Traits, template <typename CharT2, typename Traits2> class Stream,
		  std::ios_base::openmode DefaultMode>
class basic_stdio_stream;

#ifdef _MSC_VER
#	pragma pack(1)
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
class basic_stdiobuf : public std::basic_streambuf<CharT, Traits>
{
public:
	using super_type = std::basic_streambuf<CharT, Traits>;
	using char_type = CharT;
	using traits_type = Traits;
	using int_type = typename traits_type::int_type;
	using off_type = typename traits_type::off_type;
	using pos_type = typename traits_type::pos_type;

	template <typename CharT2, typename Traits2, template <typename CharT3, typename Traits3> class Stream,
			  std::ios_base::openmode DefaultMode>
	friend class basic_stdio_stream;

	basic_stdiobuf()
		: super_type{}
		, file_{ nullptr }
		, unget_buf_(traits_type::eof())
	{}

	explicit basic_stdiobuf(std::FILE* file)
		: super_type{}
		, file_{ file }
		, unget_buf_{ traits_type::eof() }
	{}

	basic_stdiobuf(basic_stdiobuf const&) = delete;

	basic_stdiobuf(basic_stdiobuf&& other)
		: super_type(std::move(other))
		, file_{ other.file_ }
		, unget_buf_{ other.unget_buf_ }
	{
		other.file_ = nullptr;
		other.unget_buf_ = traits_type::eof();
	}

	~basic_stdiobuf() override { close(); }

	basic_stdiobuf& operator=(basic_stdiobuf const&) = delete;

	basic_stdiobuf& operator=(basic_stdiobuf&& other)
	{
		super_type::operator=(std::move(other));
		if (file_ != nullptr) std::fclose(file_);
		file_ = std::exchange(other.file_, nullptr);
		unget_buf_ = std::exchange(other.unget_buf_, traits_type::eof());
		return *this;
	}

	void swap(basic_stdiobuf& other)
	{
		super_type::swap(other);
		std::swap(file_, other.file_);
		std::swap(unget_buf_, other.unget_buf_);
	}

	std::FILE* file() { return file_; }

	bool is_open() const { return file_ != nullptr; }

	void close()
	{
		if (file_ != nullptr) std::fclose(file_);
		file_ = nullptr;
	}

protected:
	int_type syncgetc();

	int_type syncungetc(int_type c);

	int_type syncputc(int_type c);

	int_type underflow() override { return this->syncungetc(this->syncgetc()); }

	int_type uflow() override { return unget_buf_ = this->syncgetc(); }

	int_type pbackfail(int_type c = traits_type::eof()) override
	{
		int_type ret;
		int_type const eof = traits_type::eof();

		// Check if the unget or putback was requested
		if (traits_type::eq_int_type(c, eof)) // unget
			if (!traits_type::eq_int_type(unget_buf_, eof))
				ret = this->syncungetc(unget_buf_);
			else // buffer invalid, fail.
				ret = eof;
		else // putback
			ret = this->syncungetc(c);

		// The buffered character is no longer valid, discard it.
		unget_buf_ = eof;
		return ret;
	}

	std::streamsize xsgetn(char_type* s, std::streamsize n) override;

	int_type overflow(int_type c = traits_type::eof()) override
	{
		int_type ret;
		if (traits_type::eq_int_type(c, traits_type::eof()))
			ret = std::fflush(file_) != 0 ? traits_type::eof() : traits_type::not_eof(c);
		else
			ret = this->syncputc(c);
		return ret;
	}

	std::streamsize xsputn(char_type const* s, std::streamsize n) override;

	int sync() override { return std::fflush(file_) == 0 ? 0 : -1; }

	std::streampos seekoff(std::streamoff off, std::ios_base::seekdir dir,
						   std::ios_base::openmode = std::ios_base::in | std::ios_base::out) override
	{
		std::streampos ret(std::streamoff(-1));
		int whence;
		if (dir == std::ios_base::beg)
			whence = SEEK_SET;
		else if (dir == std::ios_base::cur)
			whence = SEEK_CUR;
		else
			whence = SEEK_END;
#if (defined(_FILE_OFFSET_BIT) && _FILE_OFFSET_BITS == 64) || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200'112L)
		if (!::fseeko(file_, static_cast<off_t>(off), whence)) ret = std::streampos(::ftello(file_));
#elif defined(_WIN32)
		if (!::_fseeki64(file_, off, whence)) ret = std::streampos(::_ftelli64(file_));
#else
		if (!std::fseek(file_, off, whence)) ret = std::streampos(std::ftell(file_));
#endif
		return ret;
	}

	std::streampos seekpos(std::streampos pos,
						   std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) override
	{
		return seekoff(std::streamoff(pos), std::ios_base::beg, mode);
	}

private:
	std::FILE* private_file() const { return file_; }

	std::FILE* file_;
	int_type unget_buf_;
};

using stdiobuf = basic_stdiobuf<char>;
using wstdiobuf = basic_stdiobuf<wchar_t>;

template <>
inline auto stdiobuf::syncgetc() -> int_type
{
	return std::getc(file_);
}

template <>
inline auto stdiobuf::syncungetc(int_type c) -> int_type
{
	return std::ungetc(c, file_);
}

template <>
inline auto stdiobuf::syncputc(int_type c) -> int_type
{
	return std::putc(c, file_);
}

template <>
inline std::streamsize stdiobuf::xsgetn(char* s, std::streamsize n)
{
	std::streamsize ret = static_cast<std::streamsize>(std::fread(s, 1, static_cast<std::size_t>(n), file_));
	if (ret > 0)
		unget_buf_ = traits_type::to_int_type(s[ret - 1]);
	else
		unget_buf_ = traits_type::eof();
	return ret;
}

template <>
inline std::streamsize stdiobuf::xsputn(char const* s, std::streamsize n)
{
	return static_cast<std::streamsize>(std::fwrite(s, 1, static_cast<std::size_t>(n), file_));
}

template <>
inline auto wstdiobuf::syncgetc() -> int_type
{
	return std::getwc(file_);
}

template <>
inline auto wstdiobuf::syncungetc(int_type c) -> int_type
{
	return std::ungetwc(c, file_);
}

template <>
inline auto wstdiobuf::syncputc(int_type c) -> int_type
{
	return std::putwc(traits_type::to_char_type(c), file_);
}

template <>
inline std::streamsize wstdiobuf::xsgetn(wchar_t* s, std::streamsize n)
{
	std::streamsize ret = 0;
	int_type const eof = traits_type::eof();
	while (n--)
	{
		int_type c = this->syncgetc();
		if (traits_type::eq_int_type(c, eof)) break;
		s[ret] = traits_type::to_char_type(c);
		++ret;
	}

	if (ret > 0)
		unget_buf_ = traits_type::to_int_type(s[ret - 1]);
	else
		unget_buf_ = traits_type::eof();
	return ret;
}

template <>
inline std::streamsize wstdiobuf::xsputn(wchar_t const* s, std::streamsize n)
{
	std::streamsize ret = 0;
	int_type const eof = traits_type::eof();
	while (n--)
	{
		if (traits_type::eq_int_type(this->syncputc(traits_type::to_int_type(*s++)), eof)) break;
		++ret;
	}
	return ret;
}

#if defined(INF_HEADER_ONLY) || defined(INF_STATIC_STDIOBUF)
#else
extern template class basic_stdiobuf<char>;
extern template class basic_stdiobuf<wchar_t>;
#endif

} // namespace inf
