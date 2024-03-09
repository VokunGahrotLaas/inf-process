// STL
#include <array>
#include <future>
#include <random>
// inf network
#include <inf/network.hpp>
// inf
#include <inf/stdio_stream.hpp>

void main_server(std::string_view port);
void main_client(std::string_view port);

int main(void)
{
	inf::init_socket();

	std::mt19937 rnd{ std::random_device{}() };
	std::uniform_int_distribution<int> dist{ 50'000, 60'000 };
	std::string port = std::to_string(dist(rnd));
	inf::cout << "running on port " << port << std::endl;

	auto server = std::async(std::launch::async, main_server, port);
	main_client(port);
	server.wait();

	return 0;
}

void main_server(std::string_view port)
{
	auto socket = inf::TCPServerSocket::listen("localhost", port, 1);
	auto client = socket.accept();
	socket.close();
	std::array<char, 4'096> buffer;
	size_t nread = client.read({ buffer.data(), buffer.size() - 1 });
	buffer[nread] = '\0';
	inf::cout.lock() << "server recieved: " << buffer.data() << std::endl;
	client.write("Hello Client!");
	inf::cout.lock() << "server sent" << std::endl;
}

void main_client(std::string_view port)
{
	auto socket = inf::TCPClientSocket::connect("localhost", port);
	socket.write("Hello Server!");
	inf::cout.lock() << "client sent" << std::endl;
	std::array<char, 4'096> buffer;
	size_t nread = socket.read({ buffer.data(), buffer.size() - 1 });
	buffer[nread] = '\0';
	inf::cout.lock() << "client recieved: " << buffer.data() << std::endl;
}
