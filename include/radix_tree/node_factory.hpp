#ifndef RADIX_TREE_NODE_FACTORY_HPP
#define RADIX_TREE_NODE_FACTORY_HPP

#include <memory>
#include <vector>

namespace radix_tree {

template<typename NodeT>
class NodeFactory
{
public:
    NodeT &newNode(typename NodeT::Key const& key)
    {
        m_store.push_back(std::make_unique<NodeT>(key));
        return *m_store.back();
    }

    void clear()
    {
        m_store.clear();
    }

private:
    std::vector<std::unique_ptr<NodeT>> m_store;
};

} // namespace radix_tree

#endif // RADIX_TREE_NODE_FACTORY_HPP
