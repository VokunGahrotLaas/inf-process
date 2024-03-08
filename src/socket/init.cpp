#ifdef _WIN32

// STL
#	include <iostream>
// Windows
#	include <windef.h>
// inf
#	include <inf/network.hpp>

namespace inf
{

int err = 0;

[[gnu::constructor]]
void init_network()
{
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
}

[[gnu::destructor]]
void fini_network()
{
	if (err != 0)
	{
		std::cerr << "Winsock 2.2 dll dtor err" << std::endl;
		return;
	}

	WSACleanup();
}

} // namespace inf

#endif
