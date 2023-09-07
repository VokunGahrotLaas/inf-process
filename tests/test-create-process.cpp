#ifdef _WIN32

// STL
#	include <iomanip>
#	include <iostream>
#	include <string>
// CreateProcess
#	include <windows.h>t
// inf
#	include <inf/pipe.hpp>

int main()
{
	using namespace std::string_literals;

	bool is_parent = argc == 1;
	bool is_child = argc == 2 && argv[1] == "child"s;

	if (!is_parent && !is_child)
	{
		printf("Usage: %s [child]\n", argv[0]);
		return 1;
	}

	auto pipe = inf::make_pipe();

	if (is_child)
	{
		pipe.close_read();
		pipe.write() << "Hello World!" << std::endl;
		return 0;
	}

	pipe.write_close();

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process.
	if (!CreateProcess(NULL, // No module name (use command line)
					   std::quoted(argv[0]) + " child"s, // Command line
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
