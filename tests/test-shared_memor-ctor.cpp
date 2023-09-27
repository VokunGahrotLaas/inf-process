// inf
#include <inf/shared_memory.hpp>
#include <inf/stdio_stream.hpp>

class Dummy
{
public:
	Dummy()
		: id_{ count_++ }
	{
		inf::cout << id_ << ": Dummy()" << std::endl;
	}
	Dummy(Dummy const& other)
		: id_{ count_++ }
	{
		inf::cout << id_ << ": Dummy(Dummy const&) from " << other.id_ << std::endl;
	}
	Dummy(Dummy&& other)
		: id_{ count_++ }
	{
		inf::cout << id_ << ": Dummy(Dummy&&) from " << other.id_ << std::endl;
	}
	~Dummy() { inf::cout << id_ << ": ~Dummy()" << std::endl; }

	Dummy& operator=(Dummy const& other)
	{
		inf::cout << id_ << ": Dummy(Dummy const&) from " << other.id_ << std::endl;
		return *this;
	}
	Dummy& operator=(Dummy&& other)
	{
		inf::cout << id_ << ": Dummy(Dummy&&) from " << other.id_ << std::endl;
		return *this;
	}

	friend std::ostream& operator<<(std::ostream& os, Dummy const& dummy)
	{
		return os << "(Dummy " << dummy.id_ << ')';
	}

private:
	static std::size_t count_;
	std::size_t id_;
};

std::size_t Dummy::count_{ 0 };

int main()
{
	inf::shared_memory shm{ 4'096 };
	auto map0 = shm.map();
	auto map1 = shm.map();
	shm.close();

	auto& map0d0 = map0.construct<Dummy>();
	auto& map1d0 = map1.allocate<Dummy>();

	auto& map1d1 = map1.construct<Dummy>();
	auto& map0d1 = map0.allocate<Dummy>();

	auto& map0d2 = map0.construct<Dummy>();

	inf::cout << "map0d0: " << map0d0 << std::endl;
	inf::cout << "map0d1: " << map0d1 << std::endl;
	inf::cout << "map0d2: " << map0d2 << std::endl;
	inf::cout << "map1d0: " << map1d0 << std::endl;
	inf::cout << "map1d1: " << map1d1 << std::endl;

	map1.release(map1d1);
	map0.acquire(map0d1);
	map0.destroy(map0d0);

	return 0;
}
