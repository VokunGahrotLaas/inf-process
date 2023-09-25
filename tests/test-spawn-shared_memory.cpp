// STL
#include <algorithm>
#include <string>
#include <string_view>
// inf
#include <inf/shared_memory.hpp>
#include <inf/spawn.hpp>
#include <inf/stdio_stream.hpp>

int int_of_str(std::string_view str) { return std::atoi(str.data()); }

std::size_t size_of_str(std::string_view str) { return static_cast<std::size_t>(std::atoll(str.data())); }

int main(int argc, char** argv)
{
	using namespace std::string_literals;

	bool is_parent = argc == 1;
	bool is_child = argc == 3;

	if (!is_parent && !is_child)
	{
		inf::cerr << "Usage: " << argv[0] << " [<handle> <size>]" << std::endl;
		return 1;
	}

	if (is_child)
	{
		auto shm = inf::shared_memory::from_fd(int_of_str(argv[1]), size_of_str(argv[2]));
		auto map = shm.map<char>();
		shm.close();
		std::string_view str = "Hello World!";
		std::copy(str.begin(), str.end(), map.data());
		return 0;
	}

	inf::shared_memory shm{ 4'096 };

	std::string handle = std::to_string(shm.fd());
	std::string size = std::to_string(shm.size());
	char* child_argv[] = { argv[0], handle.data(), size.data(), nullptr };
	inf::spawn spawn{ argv[0], child_argv, nullptr };

	auto map = shm.map<char>();
	shm.close();
	spawn.wait_exit();

	inf::cout << "mmap: " << std::string_view{ map.data() } << std::endl;

	return 0;
}
