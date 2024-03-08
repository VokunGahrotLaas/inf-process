// STL
#include <array>
#include <iostream>
// inf network
#include <inf/network.hpp>

int main(int argc, char** argv)
{
	if (argc != 2) return 1;
	std::string_view const port = argv[1];
	auto server = inf::TCPServerSocket::listen("0.0.0.0", port, 10);
	std::cout << "Running on port " << port << std::endl;
	auto client = server.accept();
	std::cout << "Accepted client" << std::endl;
	client.write("Hello client!");
	std::array<char, 4'096> buffer;
	size_t size = client.read({ buffer.data(), buffer.size() - 1 });
	buffer[size] = '\0';
	std::cout << "Recieved: " << buffer.data() << std::endl;
	return 0;
}
