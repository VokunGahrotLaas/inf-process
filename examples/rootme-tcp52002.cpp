// STL
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
// inf network
#include <inf/network.hpp>

int main()
{
	inf::init_socket();
	constexpr std::string_view addr = "challenge01.root-me.org";
	constexpr std::string_view port = "52002";
	auto client = inf::TCPClientSocket::connect(addr, port);
	std::cout << "Connected to " << addr << ':' << port << std::endl;
	std::array<char, 4'096> buffer;
	size_t size = client.read({ buffer.data(), buffer.size() - 1 });
	buffer[size] = '\0';
	std::cout << "Recieved: " << buffer.data() << std::endl;

	double a;
	double b;
	{
		std::istringstream iss{ buffer.data() };
		std::string tmp;
		for (int i = 0; i < 5; ++i)
			std::getline(iss, tmp);
		iss >> tmp >> tmp >> tmp >> tmp >> tmp >> a >> tmp >> tmp >> tmp >> b;
	}

	double res = std::sqrt(a) * b;
	std::cout << "sqrt(" << a << ") * " << b << " = " << res << std::endl;

	std::ostringstream oss;
	oss << std::setprecision(2) << std::fixed << res << '\n';
	std::cout << "res: " << oss.str() << std::endl;
	client.write(oss.str());
	size = client.read({ buffer.data(), buffer.size() - 1 });
	buffer[size] = '\0';
	std::cout << "Recieved: " << buffer.data() << std::endl;

	return 0;
}
