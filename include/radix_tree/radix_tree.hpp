#ifndef RADIX_TREE_HPP
#define RADIX_TREE_HPP

#include "node.hpp"
#include "node_factory.hpp"

namespace radix_tree {

template<typename Key, typename Value>
class tree
{
public:
    using key_t = Key;
    using value_t = Value;
    using node_t = node<Key, Value>;

public:
    // modifier
    void insert(Key const&, Value const&);
    void clear();

    // query
    size_t node_count() const;
    size_t value_count() const;

    // Traverse tree with given key and call visitor on each nodes in the path.
    //
    // @param Visitor callable that has signature of void(node_t const&, Key node_key)
    template<typename Visitor>
        void traverse(Key const&, Visitor&&) const;

    // Traverse all nodes in the tree and call visitor on each nodes.
    //
    // @param Visitor callable which has signature void(node_t const&, size_t level)
    template<typename Visitor>
        void traverse_all(Visitor&&) const;

    void validate() const
    {
        m_root.traverse(
            [](auto& node, size_t) {
                node.validate();
            }
        );
    }

private:
    node_factory<node_t> m_factory;
    node_t m_root;
};

} // namespace radix_tree

#include "radix_tree.tcc"

#endif // RADIX_TREE_HPP

