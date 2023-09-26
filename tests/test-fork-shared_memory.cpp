#ifndef _WIN32
// STL
#	include <algorithm>
#	include <string_view>
// inf
#	include <inf/fork.hpp>
#	include <inf/shared_memory.hpp>
#	include <inf/stdio_stream.hpp>

int main(void)
{
	inf::shared_memory shm{ 4'096 };
	auto map = shm.map();
	shm.close();

	inf::fork fork;

	if (fork.is_child())
	{
		std::string_view str = "Hello World!";
		auto size = map.construct<size_t>(str.size() + 1);
		auto chars = map.construct_array<char>(size);
		std::copy(str.begin(), str.end(), chars.begin());
		chars.back() = '\0';
		return 0;
	}

	fork.wait_exit();

	auto size = map.allocate<size_t>();
	auto chars = map.allocate_array<char>(size);
	inf::cout << "mmap: " << std::string_view{ chars.data(), chars.size() } << std::endl;

	return 0;
}
#else
#	include <inf/stdio_stream.hpp>

int main()
{
	inf::cout << "cannot fork so this test has no counterpart on windows" << std::endl;
	return 0;
}
#endif
