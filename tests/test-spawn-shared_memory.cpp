// STL
#include <algorithm>
#include <string>
#include <string_view>
// inf
#include <inf/shared_memory.hpp>
#include <inf/spawn.hpp>
#include <inf/stdio_stream.hpp>

std::size_t size_of_str(std::string_view str) { return static_cast<std::size_t>(std::atoll(str.data())); }

std::intptr_t intptr_of_str(std::string_view str) { return std::atoll(str.data()); }

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
		auto shm = inf::shared_memory::from_native_handle(intptr_of_str(argv[1]), size_of_str(argv[2]));
		auto map = shm.map();
		shm.close();
		std::string_view str = "Hello World!";
		auto size = map.construct<size_t>(str.size() + 1);
		auto chars = map.construct_array<char>(size);
		std::copy(str.begin(), str.end(), chars.begin());
		chars.back() = '\0';
		return 0;
	}

	inf::shared_memory shm{ 4'096 };

	std::string handle = std::to_string(shm.native_handle());
	std::string shm_size = std::to_string(shm.size());
	char* child_argv[] = { argv[0], handle.data(), shm_size.data(), nullptr };
	inf::spawn spawn{ argv[0], child_argv, nullptr };

	auto map = shm.map();
	shm.close();
	spawn.wait_exit();

	auto size = map.allocate<size_t>();
	auto chars = map.allocate_array<char>(size);
	inf::cout << "mmap: " << std::string_view{ chars.data(), chars.size() } << std::endl;

	return 0;
}
