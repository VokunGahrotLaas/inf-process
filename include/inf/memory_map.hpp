#pragma once

// STL
#include <functional>
#include <memory>
#include <span>
#include <vector>
// Unix / Windows
#ifndef _WIN32
#	include <sys/mman.h>
#else
#	include <memoryapi.h>
#endif
// inf
#include <inf/errno_guard.hpp>
#include <inf/ioutils.hpp>

namespace inf
{

class memory_map
{
public:
	explicit memory_map(intptr_t handle, std::size_t size, ::off_t offset = 0,
						inf::source_location location = inf::source_location::current())
		: map_{ nullptr }
		, capacity_{ size }
		, size_{ 0 }
	{
#ifndef _WIN32
		errno_guard errg{ "mmap" };
		map_ = static_cast<decltype(map_)>(
			::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, static_cast<int>(handle), offset));
		if (map_ == MAP_FAILED) errg.throw_error(location);
#else
		errno_guard errg{ "MapViewOfFile" };
		map_ = static_cast<decltype(map_)>(
			::MapViewOfFile(reinterpret_cast<HANDLE>(handle), FILE_MAP_ALL_ACCESS, 0, offset, size));
		if (map_ == nullptr) errg.throw_error(location);
#endif
	}

	memory_map(memory_map const&) = delete;
	memory_map& operator=(memory_map const&) = delete;

	memory_map(memory_map&&) = default;
	memory_map& operator=(memory_map&&) = default;

	~memory_map() { close(); }

	void close(inf::source_location location = inf::source_location::current())
	{
		for (auto& deleter: deleters_)
			deleter();
		deleters_.clear();
		if (map_ != nullptr)
		{
#ifndef _WIN32
			errno_guard errg{ "munmap" };
			if (::munmap(map_, capacity_) < 0) errg.throw_error(location);
#else
			errno_guard errg{ "UnmapViewOfFile" };
			if (!::UnmapViewOfFile(map_)) errg.throw_error(location);
#endif
		}
	}

	inline void* data() { return map_; }
	inline void const* data() const { return map_; }

	inline std::size_t size() const { return size_; }
	inline std::size_t capacity() const { return capacity_; }

	template <typename T>
	T& allocate(source_location location = source_location::current())
	{
		void* ptr = map_ + size_;
		std::size_t size = capacity_ - size_;
		if (std::align(alignof(T), sizeof(T), ptr, size) == nullptr) throw exception("out of bound", location);
		size_ += sizeof(T) + (capacity_ - size_ - size);
		return *static_cast<T*>(ptr);
	}

	template <typename T, typename... Args>
	T& construct(Args&&... args)
	{
		T* ptr = &allocate<T>();
		::new (static_cast<void*>(ptr)) T(std::forward<Args>(args)...);
		deleters_.push_back([ptr]() { std::destroy_at(ptr); });
		return *ptr;
	}

	template <typename T, std::size_t E = std::dynamic_extent>
	std::span<T, E> allocate_array(std::size_t nb = 0, source_location location = source_location::current())
	{
		if (nb == 0 && E == std::dynamic_extent) throw exception("invalid arg", location);
		if (nb == 0) nb = E;
		void* ptr = map_ + size_;
		std::size_t size = capacity_ - size_;
		if (std::align(alignof(T), sizeof(T) * nb, ptr, size) == nullptr) throw exception("out of bound", location);
		size_ += sizeof(T) * nb + (capacity_ - size_ - size);
		deleters_.push_back([ptr, nb]() { std::destroy_n(static_cast<T*>(ptr), nb); });
		return std::span<T, E>{ static_cast<T*>(ptr), nb };
	}

	template <typename T, std::size_t E = std::dynamic_extent, typename... Args>
	std::span<T, E> construct_array(std::size_t nb, T const& value,
									source_location location = source_location::current())
	{
		std::span<T, E> span = allocate<T>(nb, location);
		std::uninitialized_fill(span.begin(), span.end(), value);
		return span;
	}

	template <typename T, std::size_t E = std::dynamic_extent, typename... Args>
	std::span<T, E> construct_array(std::size_t nb = 0, source_location location = source_location::current())
	{
		std::span<T, E> span = allocate_array<T>(nb, location);
		std::uninitialized_default_construct(span.begin(), span.end());
		return span;
	}

private:
	std::byte* map_{ nullptr };
	std::size_t capacity_{ 0 };
	std::size_t size_{ 0 };
	std::vector<std::function<void()>> deleters_{};
};

} // namespace inf
