#ifndef _WIN32

// STL
#	include <iostream>
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

	std::cout << "pipe: " << line << std::endl;

	return 0;
}

#else

#	include <iostream>

int main()
{
	std::cerr << "cannot fork so this test has no counterpart on windows" << std::endl;
	return 0;
}

#endif
