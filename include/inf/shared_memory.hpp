#pragma once

// STL
#include <memory>
#include <span>
// Unix / Windows
#ifndef _WIN32
#	include <sys/mman.h>
#	include <unistd.h>
#else
#	include <memoryapi.h>
#endif
// inf
#include <inf/errno_guard.hpp>
#include <inf/exceptions.hpp>
#include <inf/ioutils.hpp>
#include <inf/memory_map.hpp>

namespace inf
{

class shared_memory
{
public:
	explicit shared_memory(std::size_t size, inf::source_location location = inf::source_location::current())
	{
#ifndef _WIN32
		{
			errno_guard errg{ "memfd_create" };
			fd_ = ::memfd_create("inf::shared_memory", 0);
			if (fd_ < 0) errg.throw_error(location);
		}
		if (size > 0)
		{
			errno_guard errg{ "ftruncate" };
			if (ftruncate(fd_, static_cast<::off_t>(size)) < 0) errg.throw_error(location);
		}
#else
		HANDLE handle = nullptr;
		{
			errno_guard errg{ "CreateFileMappingA" };
			SECURITY_ATTRIBUTES sec_attr{ sizeof(sec_attr), nullptr, true };
			uint32_t low = size & 0xffffffffu;
			uint32_t high = size >> 32;
			handle =
				::CreateFileMappingA(INVALID_HANDLE_VALUE, &sec_attr, PAGE_READWRITE | SEC_COMMIT, high, low, nullptr);
			if (handle == nullptr) errg.throw_error(location);
		}
		fd_ = winhandle_to_fd(handle, 0, location);
#endif
		size_ = size;
	}

	~shared_memory() { close(); }

	int fd() { return fd_; }

	std::size_t size() const { return size_; }

	void close(inf::source_location location = inf::source_location::current())
	{
		if (fd_ >= 0) io::close(fd_, location);
		fd_ = -1;
		size_ = 0;
	}

	static shared_memory from_fd(int fd, std::size_t size) { return shared_memory{ fd, size }; }

	template <typename T, std::size_t E = std::dynamic_extent>
	memory_map<T, E> map(std::size_t size = npos, std::size_t offset = 0,
						 inf::source_location location = inf::source_location::current())
	{
		if (offset >= size_ || size == 0 || (size + offset > size_ ? size_ - offset : size) < sizeof(T))
			throw exception("out of bounds", location);
		return memory_map<T, E>{ fd_, (size + offset > size_ ? size_ - offset : size) / sizeof(T),
								 static_cast<::off_t>(offset * sizeof(T)), location };
	}

	static constexpr std::size_t npos = static_cast<std::size_t>(-1);

private:
	explicit shared_memory(int fd, std::size_t size)
		: fd_{ fd }
		, size_{ size }
	{}

	int fd_{ -1 };
	std::size_t size_{ 0 };
};

} // namespace inf
