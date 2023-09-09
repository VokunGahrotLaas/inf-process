// STL
#include <iostream>
// inf
#include <inf/pipe.hpp>

int main()
{
	auto pipe = inf::make_pipe();

	auto cout_bck = pipe.write.safe_dup(inf::cout);

	inf::cout << "Hello World!" << std::endl;

	std::string line;
	std::getline(pipe.read, line);

	cout_bck.dup_back(inf::cout);
	inf::cout << "pipe: " << line << std::endl;

	return 0;
}
