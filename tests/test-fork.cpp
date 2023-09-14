#ifndef _WIN32

// STL
#	include <string>
// inf
#	include <inf/pipe.hpp>
#	include <inf/fork.hpp>

int main()
{
	auto pipe = inf::make_pipe();

	inf::fork fork;

	if (fork.is_child())
	{
		pipe.read.close();
		pipe.write << "Hello World!" << std::endl;
		return 0;
	}

	pipe.write.close();

	fork.wait();

	std::string line;
	std::getline(pipe.read, line);

	inf::cout << "pipe: " << line << std::endl;

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
