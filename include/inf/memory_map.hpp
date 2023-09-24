#pragma once

// STL
#include <memory>
#include <span>
// Unix / Windows
#ifndef _WIN32
#	include <sys/mman.h>
#else
#	include <Memoryapi.h>
#endif
// inf
#include <inf/errno_guard.hpp>

namespace inf
{

template <typename T, std::size_t E = std::dynamic_extent>
class memory_map : public std::span<T, E>
{
public:
	using super_type = std::span<T, E>;

	memory_map()
		requires(E > 0 && E != std::dynamic_extent)
		: memory_map{ E }
	{}

	explicit memory_map(T const& data)
		requires(E > 0 && E != std::dynamic_extent)
		: memory_map{ E, data }
	{}

	explicit memory_map(int fd, std::size_t size, ::off_t offset = 0,
						inf::source_location location = inf::source_location::current())
		: super_type{ static_cast<T*>(nullptr), size }
	{
		errno_guard errg{ "mmap" };
		void* ptr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
		if (ptr == MAP_FAILED) errg.throw_error(location);
		super_type::operator=(super_type{ reinterpret_cast<T*>(ptr), size });
		std::uninitialized_default_construct(super_type::begin(), super_type::end());
	}

	explicit memory_map(int fd, std::size_t size, T const& data, ::off_t offset = 0,
						inf::source_location location = inf::source_location::current())
		: super_type{ static_cast<T*>(nullptr), size }
	{
		errno_guard errg{ "mmap" };
		void* ptr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
		if (ptr == MAP_FAILED) errg.throw_error(location);
		super_type::operator=(super_type{ reinterpret_cast<T*>(ptr), size });
		std::uninitialized_fill(super_type::begin(), super_type::end(), data);
	}

	memory_map(memory_map const&) = delete;
	memory_map& operator=(memory_map const&) = delete;

	memory_map(memory_map&&) = default;
	memory_map& operator=(memory_map&&) = default;

	~memory_map() { close(); }

	void close(inf::source_location location = inf::source_location::current())
	{
		if (super_type::data() != nullptr)
		{
			errno_guard errg{ "munmap" };
			if (::munmap(super_type::data(), super_type::size()) < 0) errg.throw_error(location);
		}
		super_type::operator=(super_type{ static_cast<T*>(nullptr), super_type::size() });
	}
};

} // namespace inf
