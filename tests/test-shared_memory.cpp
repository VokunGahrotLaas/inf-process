// inf
#include <inf/shared_memory.hpp>
#include <inf/stdio_stream.hpp>

int main()
{
	inf::shared_memory shm{ 4'096 };
	auto map0 = shm.map<int>();
	auto map1 = shm.map<int>();
	shm.close();
	map0[0] = 42;
	inf::cout << "map1: " << map1[0] << std::endl;
	map1[0] = 24;
	inf::cout << "map0: " << map0[0] << std::endl;
	return 0;
}
