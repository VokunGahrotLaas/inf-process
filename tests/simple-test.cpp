// STL
#include <iostream>
// inf
#include <inf/pipe.hpp>

int main()
{
	auto pipe = inf::make_pipe();

	pipe.write() << "Hello World!\n";

	std::string line;
	std::getline(pipe.read(), line);

	std::cout << line << std::endl;

	return 0;
}
