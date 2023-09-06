#include <inf/stdio_stream.hpp>

namespace inf
{

template class basic_stdio_stream<char, std::char_traits<char>, std::basic_iostream,
								  std::ios_base::in | std::ios_base::out>;
template class basic_stdio_stream<wchar_t, std::char_traits<wchar_t>, std::basic_iostream,
								  std::ios_base::in | std::ios_base::out>;

template class basic_stdio_stream<char, std::char_traits<char>, std::basic_istream, std::ios_base::in>;
template class basic_stdio_stream<wchar_t, std::char_traits<wchar_t>, std::basic_istream, std::ios_base::in>;

template class basic_stdio_stream<char, std::char_traits<char>, std::basic_ostream, std::ios_base::out>;
template class basic_stdio_stream<wchar_t, std::char_traits<wchar_t>, std::basic_ostream, std::ios_base::out>;

} // namespace inf
