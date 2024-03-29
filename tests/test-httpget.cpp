// STL
#include <array>
#include <iostream>
// inf network
#include <inf/network.hpp>

#define MESSAGE                                                                                                        \
	"GET /random_joke HTTP/1.1\nHost: official-joke-api.appspot.com\nAccept: "                                         \
	"application/json;charset=UTF-8\nAccept-Encoding: identity\n\n"

int main()
{
	inf::init_socket();
	auto socket = inf::TCPClientSocket::connect("official-joke-api.appspot.com", "80");
	socket.write(MESSAGE);
	std::cout << "sent" << std::endl;
	std::array<char, 4'096 * 16> buffer;
	size_t nread = socket.read({ buffer.data(), buffer.size() - 1 });
	buffer[nread] = '\0';
	std::cout << "recieved:\n" << buffer.data() << std::endl;
	return 0;
}
