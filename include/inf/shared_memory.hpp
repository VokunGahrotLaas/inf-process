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
			handle_ = ::memfd_create("inf::shared_memory", 0);
			if (fd() < 0) errg.throw_error(location);
		}
		if (size > 0)
		{
			errno_guard errg{ "ftruncate" };
			if (ftruncate(fd(), static_cast<::off_t>(size)) < 0) errg.throw_error(location);
		}
#else
		{
			errno_guard errg{ "CreateFileMappingA" };
			SECURITY_ATTRIBUTES sec_attr{ sizeof(sec_attr), nullptr, true };
			uint32_t low = size & 0xffffffffu;
			uint32_t high = size >> 32;
			handle_ = reinterpret_cast<intptr_t>(
				::CreateFileMappingA(INVALID_HANDLE_VALUE, &sec_attr, PAGE_READWRITE | SEC_COMMIT, high, low, nullptr));
			if (handle() == nullptr) errg.throw_error(location);
		}
#endif
		size_ = size;
	}

	~shared_memory() { close(); }

	inline intptr_t native_handle() { return handle_; }
	static shared_memory from_native_handle(intptr_t handle, std::size_t size) { return shared_memory{ handle, size }; }

#ifndef _WIN32
	inline int fd() { return static_cast<int>(handle_); }
	static shared_memory from_fd(int fd, std::size_t size) { return shared_memory{ fd, size }; }
#else
	inline HANDLE handle() { return reinterpret_cast<HANDLE>(handle_); }
	static shared_memory from_handle(HANDLE handle, std::size_t size)
	{
		return shared_memory{ reinterpret_cast<intptr_t>(handle), size };
	}
#endif

	std::size_t size() const { return size_; }

	void close(inf::source_location location = inf::source_location::current())
	{
#ifndef _WIN32
		if (fd() >= 0) io::close(fd(), location);
#else
		if (handle() != INVALID_HANDLE_VALUE) io::close_handle(handle(), location);
#endif
		handle_ = -1;
		size_ = 0;
	}

	template <typename T, std::size_t E = std::dynamic_extent>
	memory_map<T, E> map(std::size_t size = npos, std::size_t offset = 0,
						 inf::source_location location = inf::source_location::current())
	{
		if (offset >= size_ || size == 0 || (size + offset > size_ ? size_ - offset : size) < sizeof(T))
			throw exception("out of bounds", location);
		return memory_map<T, E>{ handle_, (size + offset > size_ ? size_ - offset : size) / sizeof(T),
								 static_cast<::off_t>(offset * sizeof(T)), location };
	}

	static constexpr std::size_t npos = static_cast<std::size_t>(-1);

private:
	explicit shared_memory(intptr_t handle, std::size_t size)
		: handle_{ handle }
		, size_{ size }
	{}

	intptr_t handle_{ -1 };
	std::size_t size_{ 0 };
};

} // namespace inf
