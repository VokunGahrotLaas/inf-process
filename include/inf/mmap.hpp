#pragma once

// STL
#include <memory>
#include <span>
// unix
#include <sys/mman.h>
// inf
#include <inf/errno_guard.hpp>

namespace inf
{

template <typename T, std::size_t E>
class mmap : public std::span<T, E>
{
public:
	using super_type = std::span<T, E>;

	mmap()
		requires(E > 0 && E != std::dynamic_extent)
		: mmap{ E }
	{}

	mmap(T const& data)
		requires(E > 0 && E != std::dynamic_extent)
		: mmap{ E, data }
	{}

	explicit mmap(std::size_t size)
		: super_type{ static_cast<T*>(nullptr), size }
	{
		errno_guard errg{ "mmap" };
		void* ptr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		if (ptr == MAP_FAILED) errg.throw_error();
		super_type::operator=(super_type{ reinterpret_cast<T*>(ptr), size });
		std::uninitialized_default_construct(super_type::begin(), super_type::end());
	}

	explicit mmap(std::size_t size, T const& data)
		: super_type{ static_cast<T*>(nullptr), size }
	{
		errno_guard errg{ "mmap" };
		void* ptr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		if (ptr == MAP_FAILED) errg.throw_error();
		super_type::operator=(super_type{ reinterpret_cast<T*>(ptr), size });
		std::uninitialized_fill(super_type::begin(), super_type::end(), data);
	}

	~mmap()
	{
		errno_guard errg{ "munmap" };
		if (::munmap(super_type::data(), super_type::size()) < 0) errg.throw_error();
	}
};

} // namespace inf
