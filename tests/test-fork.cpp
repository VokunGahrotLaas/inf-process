#ifndef _WIN32

// STL
#	include <iostream>
#	include <string>
// unistd
#	include <sys/wait.h>
#	include <unistd.h>
// inf
#	include <inf/pipe.hpp>

int main()
{
	auto pipe = inf::make_pipe();

	pid_t pid = ::fork();

	if (pid < 0)
	{
		std::cerr << "could not fork" << std::endl;
		return 1;
	}

	if (pid == 0)
	{
		pipe.read.close();
		pipe.write << "Hello World!" << std::endl;
		return 0;
	}

	pipe.write.close();

	if (::waitpid(pid, NULL, 0) < 0)
	{
		std::cerr << "could not waitpid" << std::endl;
		return 1;
	}

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
