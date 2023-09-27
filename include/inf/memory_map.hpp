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
	explicit memory_map(intptr_t handle, std::size_t size, std::ptrdiff_t offset = 0,
						source_location location = source_location::current())
		: span_{ static_cast<std::byte*>(nullptr), 0 }
		, size_{ 0 }
	{
		void* ptr = nullptr;
#ifndef _WIN32
		errno_guard errg{ "mmap" };
		ptr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, static_cast<int>(handle), offset);
		if (ptr == MAP_FAILED) errg.throw_error(location);
#else
		errno_guard errg{ "MapViewOfFile" };
		int32_t high = 0;
		if constexpr (sizeof(offset) > sizeof(int32_t)) high = offset >> 32;
		int32_t low = offset & 0xffffffff;
		ptr = ::MapViewOfFile(reinterpret_cast<HANDLE>(handle), FILE_MAP_ALL_ACCESS, high, low, size);
		if (ptr == nullptr) errg.throw_error(location);
#endif
		span_ = std::span<std::byte>{ static_cast<std::byte*>(ptr), size };
	}

	memory_map(memory_map const&) = delete;
	memory_map& operator=(memory_map const&) = delete;

	memory_map(memory_map&&) = default;
	memory_map& operator=(memory_map&&) = default;

	~memory_map() { close(); }

	void close(source_location location = source_location::current())
	{
		for (auto [ptr, deleter]: deleters_)
			deleter();
		deleters_.clear();
		if (data() != nullptr)
		{
#ifndef _WIN32
			errno_guard errg{ "munmap" };
			if (::munmap(data(), capacity()) < 0) errg.throw_error(location);
#else
			errno_guard errg{ "UnmapViewOfFile" };
			if (!::UnmapViewOfFile(data())) errg.throw_error(location);
#endif
		}
		span_ = std::span<std::byte>{ static_cast<std::byte*>(nullptr), 0 };
	}

	inline void* data() { return span_.data(); }
	inline void const* data() const { return span_.data(); }

	inline std::size_t size() const { return size_; }
	inline std::size_t capacity() const { return span_.size(); }

	template <typename T>
	T& allocate(source_location location = source_location::current())
	{
		void* ptr = span_.data() + size();
		std::size_t space = capacity() - size();
		if (std::align(alignof(T), sizeof(T), ptr, space) == nullptr) throw exception("out of bound", location);
		size_ += sizeof(T) + (capacity() - size() - space);
		return *static_cast<T*>(ptr);
	}

	template <typename T, typename... Args>
	T& construct(Args&&... args)
	{
		T* ptr = std::addressof(allocate<T>());
		::new (static_cast<void*>(ptr)) T(std::forward<Args>(args)...);
		deleters_.insert({ ptr, [ptr]() { std::destroy_at(ptr); } });
		return *ptr;
	}

	template <typename T>
	void release(T& value, source_location location = source_location::current())
	{
		T* ptr = std::addressof(value);
		auto it = deleters_.find(ptr);
		if (it == deleters_.end()) throw exception("was not releasable", location);
		deleters_.erase(it);
	}

	template <typename T>
	void destroy(T& value, source_location location = source_location::current())
	{
		T* ptr = std::addressof(value);
		auto it = deleters_.find(ptr);
		if (it == deleters_.end()) throw exception("was not destroyable", location);
		it->second();
		deleters_.erase(it);
	}

	template <typename T>
	void acquire(T& value, source_location location = source_location::current())
	{
		T* ptr = std::addressof(value);
		if (reinterpret_cast<std::byte*>(ptr) < span_.data()
			|| span_.data() + span_.size() <= reinterpret_cast<std::byte*>(ptr))
			throw exception("out of bounds", location);
		if (!deleters_.insert({ ptr, [ptr]() { std::destroy_at(ptr); } }).second)
			throw exception("was already owned", location);
	}

	template <typename T, std::size_t E = std::dynamic_extent>
	std::span<T, E> allocate_array(std::size_t nb = 0, source_location location = source_location::current())
	{
		if (nb == 0 && E == std::dynamic_extent) throw exception("invalid arg", location);
		if (nb == 0) nb = E;
		void* ptr = span_.data() + size();
		std::size_t space = capacity() - size();
		if (std::align(alignof(T), sizeof(T) * nb, ptr, space) == nullptr) throw exception("out of bound", location);
		size_ += sizeof(T) * nb + (capacity() - size() - space);
		deleters_.insert({ ptr, [ptr, nb]() { std::destroy_n(static_cast<T*>(ptr), nb); } });
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

	template <typename T, std::size_t E>
	void release_array(std::span<T, E> value, source_location location = source_location::current())
	{
		auto it = deleters_.find(value.data());
		if (it == deleters_.end()) throw exception("was not releasable", location);
		deleters_.erase(it);
	}

	template <typename T, std::size_t E>
	void destroy_array(std::span<T, E> value, source_location location = source_location::current())
	{
		auto it = deleters_.find(value.data());
		if (it == deleters_.end()) throw exception("was not destroyable", location);
		it->second();
		deleters_.erase(it);
	}

	template <typename T, std::size_t E>
	void acquire_array(std::span<T, E> value, source_location location = source_location::current())
	{
		if (reinterpret_cast<std::byte*>(value.data()) < span_.data()
			|| span_.data() + span_.size() <= reinterpret_cast<std::byte*>(value.data()))
			throw exception("out of bounds", location);
		if (!deleters_.insert({ value.data(), [value]() { std::destroy_n(value.data(), value.size()); } }).second)
			throw exception("was already owned", location);
	}

private:
	std::span<std::byte> span_{ static_cast<std::byte*>(nullptr), 0 };
	std::size_t size_{ 0 };
	std::unordered_map<void*, std::function<void()>> deleters_{};
};

} // namespace inf
