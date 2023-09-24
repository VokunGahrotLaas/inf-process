#ifndef _WIN32

// STL
#	include <algorithm>
#	include <string_view>
#	include <thread>
// inf
#	include <inf/fork.hpp>
#	include <inf/stdio_stream.hpp>
#	include <inf/mmap.hpp>

int main()
{
	inf::mmap<char, 4'096> mmap;

	inf::fork fork;

	if (fork.is_child())
	{
		std::string_view str = "Hello World!";
		std::copy(str.begin(), str.end(), mmap.data());
		return 0;
	}

	fork.wait();

	std::string_view str{ mmap.data() };
	inf::cout << "mmap: " << str << std::endl;

	return 0;
}

#else

#	include <inf/stdio_stream.hpp>

int main()
{
	inf::cout << "cannot mmap so this test has no counterpart on windows" << std::endl;
	return 0;
}

#endif
