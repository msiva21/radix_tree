#ifndef RADIX_TREE_UTILITY_HPP
#define RADIX_TREE_UTILITY_HPP

#include <iterator>

namespace radix_tree {

template<typename It, typename N>
It
next(It const it, N const n)
{
    using diff_t = typename std::iterator_traits<It>::difference_type;

    return std::next(
        it,
        static_cast<diff_t>(n)
    );
}

} // namespace radix_tree

#endif // RADIX_TREE_UTILITY_HPP
