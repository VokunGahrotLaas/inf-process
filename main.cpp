// STL
#include <iostream>
#include <string>
// platform specific
#ifndef _MSC_VER
#	include <sys/wait.h>
#	include <unistd.h>
#endif
// pipes
#include "pipes.hpp"

int fork_main();
int simple_main();

int main() { return simple_main(); }

int fork_main()
{
	auto res = inf::make_pipe();

	if (!res)
	{
		std::cerr << "could not pipe: " << res.error() << std::endl;
		return 1;
	}

	inf::Pipe pipe = std::move(res.value());

#ifndef _MSC_VER
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
		pipe.write_close();
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
	pipe.read_close();

	std::cout << line << std::endl;
#else
	std::cout << "Not yet" << std::endl;
#endif

	return 0;
}

int simple_main()
{
	auto res = inf::make_pipe();

	if (!res)
	{
		std::cerr << "could not pipe: " << res.error() << std::endl;
		return 1;
	}

	inf::Pipe pipe = std::move(res.value());

	pipe.write() << "Hello World!\n";
	pipe.write_close();

	std::string line;
	std::getline(pipe.read(), line);
	pipe.read_close();

	std::cout << line << std::endl;

	return 0;
}
