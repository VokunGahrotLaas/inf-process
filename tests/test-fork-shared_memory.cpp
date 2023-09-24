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
	auto map = shm.map<char>();
	shm.close();

	inf::fork fork;

	if (fork.is_child())
	{
		std::string_view str = "Hello World!";
		std::copy(str.begin(), str.end(), map.data());
		return 0;
	}

	fork.wait_exit();

	inf::cout << "mmap: " << std::string_view{ map.data() } << std::endl;

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
