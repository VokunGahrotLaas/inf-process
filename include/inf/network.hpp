#pragma once

// STL
#include <cstring>
#include <iostream>
#include <span>
#include <string_view>
#include <utility>
// Unix / Windows
#ifndef _WIN32
#	include <netdb.h>
#	include <sys/socket.h>
#	include <sys/types.h>
#	include <netinet/in.h>
#else
#	include <windef.h>
#	include <winsock2.h>
#	include <ws2tcpip.h>
#endif
// inf
#include <inf/errno_guard.hpp>
#include <inf/exceptions.hpp>
#include <inf/ioutils.hpp>
#include <inf/source_location.hpp>

namespace inf
{

void init_socket();

template <int Socktype>
class Socket
{
public:
	static Socket connect(std::string_view node, std::string_view service,
						  source_location location = source_location::current())
	{
		struct addrinfo* result = gai(node, service, location);
		struct addrinfo* rp = result;
		int fd = -1;
		for (rp = result; rp != nullptr; rp = rp->ai_next)
		{
			errno_guard errg{ "socket/connect" };
			fd = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
			if (fd == -1) continue;
			if (::connect(fd, rp->ai_addr, rp->ai_addrlen) != -1) break;
			io::close(fd, location);
		}
		freeaddrinfo(result);
		if (rp == nullptr)
			throw gai_exception{ node.data(), service.data(), "could not connect to any address", location };
		return Socket{ fd };
	}

	static Socket bind(std::string_view node, std::string_view service,
					   source_location location = source_location::current())
	{
		struct addrinfo* result = gai(node, service, location);
		struct addrinfo* rp = result;
		int fd = -1;
		for (rp = result; rp != nullptr; rp = rp->ai_next)
		{
			errno_guard errg{ "socket/bind" };
			fd = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
			if (fd == -1) continue;
			if (::bind(fd, rp->ai_addr, rp->ai_addrlen) == 0) break;
			io::close(fd, location);
		}
		freeaddrinfo(result);
		if (rp == nullptr)
			throw gai_exception{ node.data(), service.data(), "could not bind to any address", location };
		return Socket{ fd };
	}

	Socket(Socket const&) = delete;
	Socket(Socket&& other)
		: fd_{ std::exchange(other.fd_, -1) }
	{}

	Socket& operator=(Socket const&) = delete;
	Socket& operator=(Socket&& other)
	{
		close();
		fd_ = std::exchange(other.fd_, -1);
		return *this;
	}

	~Socket() { close(); }

	int fd() { return fd_; }

	void write(std::span<char const> data, source_location location = source_location::current())
	{
		errno_guard errg{ "send" };
		if (::send(fd_, data.data(), data.size(), 0) < 0) errg.throw_error(location);
	}

	size_t read(std::span<char> buffer, source_location location = source_location::current())
	{
		errno_guard errg{ "recv" };
		ssize_t nrecv = ::recv(fd_, buffer.data(), buffer.size(), 0);
		if (nrecv == -1) errg.throw_error(location);
		return static_cast<size_t>(nrecv);
	}

	void close()
	{
		if (fd_ > 0)
#ifndef _WIN32
			io::close(fd_);
#else
			::closesocket(fd_);
#endif
		fd_ = -1;
	}

protected:
	explicit Socket(int fd)
		: fd_{ fd }
	{}

private:
	static struct addrinfo* gai(std::string_view node, std::string_view service, source_location location)
	{
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = Socktype;
		hints.ai_protocol = 0;
		hints.ai_flags = 0;
		struct addrinfo* result = nullptr;
		if (int ecode = ::getaddrinfo(node.data(), service.data(), &hints, &result))
			throw gai_exception(node.data(), service.data(), ::gai_strerror(ecode), location);
		return result;
	}

	int fd_;
};

extern template class Socket<SOCK_STREAM>;
extern template class Socket<SOCK_DGRAM>;

class TCPServerSocket;

class TCPClientSocket : public Socket<SOCK_STREAM>
{
public:
	using super_type = Socket<SOCK_STREAM>;
	friend TCPServerSocket;

	static TCPClientSocket connect(std::string_view node, std::string_view service,
								   source_location location = source_location::current())
	{
		return TCPClientSocket{ super_type::connect(node, service, location) };
	}

	static Socket bind(std::string_view node, std::string_view service,
					   source_location location = source_location::current()) = delete;

private:
	explicit TCPClientSocket(super_type&& super)
		: super_type{ std::move(super) }
	{}

	explicit TCPClientSocket(int fd)
		: super_type{ fd }
	{}
};

class TCPServerSocket : public Socket<SOCK_STREAM>
{
public:
	using super_type = Socket<SOCK_STREAM>;

	static TCPClientSocket connect(std::string_view node, std::string_view service, source_location location) = delete;

	static Socket bind(std::string_view node, std::string_view service, source_location location) = delete;

	static TCPServerSocket listen(std::string_view node, std::string_view service, int n,
								  source_location location = source_location::current());

	void write(std::span<char const> data, source_location location) = delete;

	size_t read(std::span<char> buffer, source_location location) = delete;

	TCPClientSocket accept(source_location location = source_location::current());

private:
	explicit TCPServerSocket(super_type&& super)
		: super_type{ std::move(super) }
	{}

	explicit TCPServerSocket(int fd)
		: super_type{ fd }
	{}
};

using UDPSocket = Socket<SOCK_DGRAM>;

#if defined(INF_HEADER_ONLY) || defined(INF_STATIC_NETWORK)

namespace
{

#	ifdef _WIN32
static int err = 0;
#	endif

void fini_socket()
{
#	ifdef _WIN32
	if (err != 0)
	{
		std::cerr << "Winsock 2.2 dll dtor err" << std::endl;
		return;
	}

	WSACleanup();
#	endif
}

} // namespace

void init_socket()
{
#	ifdef _WIN32
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0)
	{
		std::cerr << "WSAStartup failed with error: " << err << std::endl;
		std::exit(1);
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		std::cerr << "Could not find a usable version of Winsock.dll" << std::endl;
		std::exit(1);
	}
#	endif
	std::atexit(fini_socket);
}

template class Socket<SOCK_STREAM>;
template class Socket<SOCK_DGRAM>;

TCPServerSocket TCPServerSocket::listen(std::string_view node, std::string_view service, int n,
										source_location location)
{
	TCPServerSocket socket{ super_type::bind(node, service, location) };
	errno_guard errg("listen");
	if (::listen(socket.fd(), n) < 0) errg.throw_error(location);
	return socket;
}

TCPClientSocket TCPServerSocket::accept(source_location location)
{
	errno_guard errg{ "accept" };
	int cfd = ::accept(fd(), nullptr, nullptr);
	if (cfd < 0) errg.throw_error(location);
	return TCPClientSocket{ cfd };
}

#endif

} // namespace inf
