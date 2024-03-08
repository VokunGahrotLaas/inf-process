#ifdef _WIN32
// STL
#	include <iostream>
// Windows
#	include <windef.h>
#endif
// inf
#include <inf/network.hpp>

namespace inf
{

namespace
{

#ifdef _WIN32
static int err = 0;
#endif

void fini_socket()
{
#ifdef _WIN32
	if (err != 0)
	{
		std::cerr << "Winsock 2.2 dll dtor err" << std::endl;
		return;
	}

	WSACleanup();
#endif
}

} // namespace

void init_socket()
{
#ifdef _WIN32
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
#endif
	std::atexit(fini_socket);
}

} // namespace inf
