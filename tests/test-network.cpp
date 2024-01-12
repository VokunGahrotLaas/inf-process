// STL
#include <array>
#include <iostream>
// inf
#include <inf/network.hpp>

#define MESSAGE                                                                                                        \
	"GET /random_joke HTTP/1.1\nHost: official-joke-api.appspot.com\nAccept: "                                         \
	"application/json;charset=UTF-8\nAccept-Encoding: identity\n\n"

int main()
{
	auto socket = inf::TCPSocket::connect("official-joke-api.appspot.com", "80");
	socket.write(MESSAGE);
	std::cout << "sent" << std::endl;
	std::array<char, 4'096 * 16> buffer = { 0 };
	size_t nread = socket.read({ buffer.data(), buffer.size() - 1 });
	buffer[nread] = '\0';
	std::cout << "recieved:\n" << buffer.data() << std::endl;
	return 0;
}
