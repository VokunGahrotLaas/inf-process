// STL
#include <iostream>
// inf
#include <inf/pipe.hpp>

int main()
{
	auto pipe = inf::make_pipe();

	auto dup_read = pipe.read.safe_dup();

	pipe.write << "Hello World!" << std::endl;

	std::string line;
	std::getline(dup_read, line);

	std::cout << "pipe: " << line << std::endl;

	return 0;
}
