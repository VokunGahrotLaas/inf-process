#ifdef _WIN32

// STL
#	include <iomanip>
#	include <iostream>
#	include <sstream>
#	include <string>
#	include <cstdlib>
// CreateProcess
#	include <windows.h>
// inf
#	include <inf/pipe.hpp>

int main(int argc, char** argv)
{
	using namespace std::string_literals;

	bool is_parent = argc == 1;
	bool is_child = argc == 3;

	if (!is_parent && !is_child)
	{
		printf("Usage: %s [child]\n", argv[0]);
		return 1;
	}

	if (is_child)
	{
		int read = std::atoi(argv[1]);
		int write = std::atoi(argv[2]);
		inf::pipe pipe{};
		pipe.read_close();
		pipe.write() << "Hello World!" << std::endl;
		return 0;
	}

	auto pipe = inf::make_pipe();

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	std::ostringstream oss;
	oss << std::quoted(argv[0]) << ' ' << pipe.read().fd() << ' ' << pipe.write().fd();
	std::string cmd = std::move(oss).str();

	// Start the child process.
	if (!CreateProcess(NULL, // No module name (use command line)
					   cmd.data(), // Command line
					   NULL, // Process handle not inheritable
					   NULL, // Thread handle not inheritable
					   FALSE, // Set handle inheritance to FALSE
					   0, // No creation flags
					   NULL, // Use parent's environment block
					   NULL, // Use parent's starting directory
					   &si, // Pointer to STARTUPINFO structure
					   &pi) // Pointer to PROCESS_INFORMATION structure
	)
	{
		std::cerr << "CreateProcess failed (" << GetLastError() << ")." << std::endl;
		return 1;
	}

	pipe.write_close();

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles.
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	std::string line;
	std::getline(pipe.read(), line);

	std::cout << line << std::endl;

	return 0;
}

#else

#	include <iostream>

int main()
{
	std::cerr << "cannot CreateProccess so this test has no counterpart on linux" << std::endl;
	return 0;
}

#endif
