# stdio-stream

C++ wrapper for C io / POSIX / Windows functions.

Currently supports:

* Wrapping a FILE* in a inf::stdio_stream.

* Creating a inf::stdio_stream from a POSIX fd / Windows HANDLE.

* Using dup() and dup2() on inf::stdio_stream.

* Wrapping a pipe() in a inf::pipe.

## Credits / Sources

* GNU's libstdc++: [stdio_filebuf](https://gcc.gnu.org/onlinedocs/gcc-13.1.0/libstdc++/api/a12263.html)
* GNU's libstdc++: [stdio_sync_filebuf](https://gcc.gnu.org/onlinedocs/gcc-13.1.0/libstdc++/api/a12267.html)
* LLVM's libc++: [std_stream](https://github.com/llvm/llvm-project/blob/main/libcxx/src/std_stream.h)
* Nicolai M. Josuttis: [fdstream](http://www.josuttis.com/cppcode/fdstream.html)
