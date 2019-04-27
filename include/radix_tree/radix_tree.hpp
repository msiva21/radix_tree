#ifndef RADIX_TREE_HPP
#define RADIX_TREE_HPP

#include "node.hpp"
#include "node_factory.hpp"

namespace radix_tree {

template<typename K, typename V>
class RadixTree
{
public:
    using Key = K;
    using Value = V;
    using NodeType = Node<Key, Value>;

public:
    // modifier
    void insert(Key const&, Value const&);
    void clear();

    // query
    size_t node_count() const;
    size_t value_count() const;

    // Traverse tree with given key and call visitor on each nodes in the path.
    //
    // @param Visitor callable that has signature of void(NodeType const&, Key node_key)
    template<typename Visitor>
        void traverse(Key const&, Visitor&&) const;

    // Traverse all nodes in the tree and call visitor on each nodes.
    //
    // @param Visitor callable which has signature void(NodeType const&, size_t level)
    template<typename Visitor>
        void traverse_all(Visitor&&) const;

    void validate() const
    {
        m_root.traverse(
            [](NodeType const& node, size_t) {
                node.validate();
            }
        );
    }

private:
    NodeFactory<NodeType> m_factory;
    NodeType m_root;
};

} // namespace radix_tree

#include "radix_tree.tcc"

#endif // RADIX_TREE_HPP

