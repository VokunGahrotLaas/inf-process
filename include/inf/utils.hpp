#pragma once

// STL
#include <functional>
#include <sstream>
#include <string>
#include <version>

#ifndef _MSC_VER
#	define INF_GNU_PURE [[gnu::pure]]
#else
#	define INF_GNU_PURE
#endif

#define INF_STR(X) #X
#define INF_STRX(X) INF_STR(X)

#define INF_JOIN(X, Y) X##Y
#define INF_JOINX(X, Y) INF_JOIN(X, Y)

#define INF_AT_SCOPE_END(...) ::inf::at_scope_end INF_JOINX(inf_at_scope_end_, __LINE__){ [&]() { __VA_ARGS__; } };

namespace inf
{

class at_scope_end
{
public:
	at_scope_end(std::function<void()> func)
		: func_{ func }
	{}
	~at_scope_end() { func_(); }

private:
	std::function<void()> func_;
};

template <typename... Args>
std::string string_of_stream(Args&&... args)
{
	std::stringstream ss;
	(ss << ... << std::forward<Args>(args));
	return std::move(ss).str();
}

} // namespace inf
