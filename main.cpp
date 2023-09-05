// STL
#include <iostream>
#include <string>
// platform specific
#include <sys/wait.h>
#include <unistd.h>
// pipes
#include "pipes.hpp"

int fork_main();
int simple_main();

int main() { return simple_main(); }

int fork_main()
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
		pipe.read_close();
		pipe.write() << "Hello World!\n";
		return 0;
	}

	if (::waitpid(pid, NULL, 0) < 0)
	{
		std::cerr << "could not waitpid" << std::endl;
		return 1;
	}

	std::string line;

	pipe.write_close();
	std::getline(pipe.read(), line);

	std::cout << line << std::endl;

	return 0;
}

int simple_main()
{
	auto pipe = inf::make_pipe();

	pipe.write() << "Hello World!\n";

	std::string line;
	std::getline(pipe.read(), line);

	std::cout << line << std::endl;

	return 0;
}
