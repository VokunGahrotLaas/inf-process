# stdio-stream

C++ wrapper for C io / POSIX / Windows functions.

Currently supports:

* Wrapping a fd / HANDLE / FILE* in a inf::stdio_\[i]\[o]stream.

* Wrapping a pipe() / _pipe() in a inf::pipe;


## Credits / Sources

* GNU's libstdc++: [stdio_filebuf](https://gcc.gnu.org/onlinedocs/gcc-13.1.0/libstdc++/api/a12263.html)
* GNU's libstdc++: [stdio_sync_filebuf](https://gcc.gnu.org/onlinedocs/gcc-13.1.0/libstdc++/api/a12267.html)
* LLVM's libc++: [std_stream](https://github.com/llvm/llvm-project/blob/main/libcxx/src/std_stream.h)
* Nicolai M. Josuttis: [fdstream](http://www.josuttis.com/cppcode/fdstream.html)
