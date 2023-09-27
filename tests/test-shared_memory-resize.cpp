#ifndef _WIN32
// inf
#	include <inf/shared_memory.hpp>
#	include <inf/stdio_stream.hpp>

int main()
{
	inf::shared_memory shm{ 4'096 };
	auto map0 = shm.map();
	shm.resize(2'048);
	auto map1 = shm.map();
	shm.resize(8'192);
	auto map2 = shm.map();
	shm.close();
	auto& i0 = map0.construct<int>(42);
	auto& i1 = map1.allocate<int>();
	auto& i2 = map2.allocate<int>();
	inf::cout << "map1: " << i1 << std::endl;
	inf::cout << "map2: " << i2 << std::endl;
	i1 = 24;
	inf::cout << "map0: " << i0 << std::endl;
	inf::cout << "map2: " << i2 << std::endl;
	return 0;
}
#else
#	include <inf/stdio_stream.hpp>

int main()
{
	inf::cout << "Cannot resize anonymous mapping on windows yet." << std::endl;
	return 0;
}
#endif
