// STL
#include <iostream>
#include <string>
// inf
#include <inf/pipe.hpp>
#include <inf/spawn.hpp>

int main(int argc, char** argv)
{
	using namespace std::string_literals;

	bool is_parent = argc == 1;
	bool is_child = argc == 2 && argv[1] == "child"s;

	if (!is_parent && !is_child)
	{
		inf::cerr << "Usage: " << argv[0] << " [child]" << std::endl;
		return 1;
	}

	if (is_child)
	{
		inf::cout << "Hello World!" << std::endl;
		return 0;
	}

	auto pipe = inf::make_pipe();

	auto cout_bck = pipe.write.safe_dup(inf::cout);

	char child[] = "child";
	char* child_argv[] = { argv[0], child, nullptr };
	inf::spawn spawn{ argv[0], child_argv, nullptr };

	cout_bck.dup_back(inf::cout);

	spawn.wait();

	std::string line;
	std::getline(pipe.read, line);

	inf::cout << "pipe: " << line << std::endl;

	return 0;
}
