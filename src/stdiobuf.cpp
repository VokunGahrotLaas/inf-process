#define INF_STATIC_STDIOBUF
#include <inf/stdiobuf.hpp>

namespace inf
{

template class basic_stdiobuf<char>;
template class basic_stdiobuf<wchar_t>;

} // namespace inf
