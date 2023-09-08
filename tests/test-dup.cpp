// STL
#include <iostream>
// inf
#include <inf/pipe.hpp>

int main()
{
	auto pipe = inf::make_pipe();

	auto dup_read = pipe.read().dup();
	dup_read.tie(&pipe.write());
	pipe.read_close();

	pipe.write() << "Hello World!\n";

	std::string line;
	std::getline(dup_read, line);

	std::cout << "pipe: " << line << std::endl;

	return 0;
}
