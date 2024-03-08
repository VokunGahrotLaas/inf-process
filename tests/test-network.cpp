// STL
#include <array>
#include <random>
// inf network
#include <inf/network.hpp>
// inf
#include <inf/spawn.hpp>
#include <inf/stdio_stream.hpp>

int main_server(std::string_view port);
int main_client(std::string_view port);

int main(int argc, char** argv)
{
	using namespace std::string_literals;

	bool is_parent = argc == 1;
	bool is_child = argc == 2;

	if (!is_parent && !is_child)
	{
		inf::cerr << "Usage: " << argv[0] << " [port]" << std::endl;
		return 1;
	}

	if (is_child) return main_client(argv[1]);

	std::mt19937 rnd{ std::random_device{}() };
	std::uniform_int_distribution<int> dist{ 8'000, 9'000 };
	std::string port = std::to_string(dist(rnd));
	inf::cout << "running on port " << port << std::endl;
	char* child_argv[] = { argv[0], port.data(), nullptr };
	inf::spawn spawn{ argv[0], child_argv, nullptr };

	int r = main_server(port);

	spawn.wait_exit();

	return r;
}

int main_server(std::string_view port)
{
	auto socket = inf::TCPServerSocket::listen("localhost", port, 1);
	auto client = socket.accept();
	socket.close();
	std::array<char, 4'096 * 16> buffer = { 0 };
	size_t nread = client.read({ buffer.data(), buffer.size() - 1 });
	buffer[nread] = '\0';
	inf::cout << "server recieved:\n" << buffer.data() << std::endl;
	client.write("Hello Client!");
	inf::cout << "server sent" << std::endl;
	return 0;
}

int main_client(std::string_view port)
{
	auto socket = inf::TCPClientSocket::connect("localhost", port);
	socket.write("Hello Server!");
	inf::cout << "client sent" << std::endl;
	std::array<char, 4'096 * 16> buffer = { 0 };
	size_t nread = socket.read({ buffer.data(), buffer.size() - 1 });
	buffer[nread] = '\0';
	inf::cout << "client recieved:\n" << buffer.data() << std::endl;
	return 0;
}
