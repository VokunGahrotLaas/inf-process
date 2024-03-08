// STL
#include <iostream>
// inf network
#include <inf/network.hpp>

int main(int argc, char** argv)
{
	inf::init_socket();
	if (argc != 3) return 1;
	std::string_view const addr = argv[1];
	std::string_view const port = argv[2];
	auto client = inf::TCPClientSocket::connect(addr, port);
	std::cout << "Connected to " << addr << ':' << port << std::endl;
	std::array<char, 4'096> buffer;
	size_t size = client.read({ buffer.data(), buffer.size() - 1 });
	buffer[size] = '\0';
	std::cout << "Recieved: " << buffer.data() << std::endl;
	client.write("Hello server!");
	return 0;
}
