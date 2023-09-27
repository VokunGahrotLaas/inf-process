// inf
#include <inf/shared_memory.hpp>
#include <inf/stdio_stream.hpp>

int main()
{
	inf::shared_memory shm{ 4'096 };
	auto map0 = shm.map();
	auto map1 = shm.map();
	shm.close();
	auto& i0 = map0.construct<int>(42);
	auto& i1 = map1.allocate<int>();
	inf::cout << "map1: " << i1 << std::endl;
	i1 = 24;
	inf::cout << "map0: " << i0 << std::endl;
	return 0;
}
