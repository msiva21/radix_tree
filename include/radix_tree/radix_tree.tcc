#include "radix_tree.hpp"

#include "utility.hpp"

namespace radix_tree {

template<typename K, typename V>
void RadixTree<K, V>::
insert(Key const& key, Value const& value)
{
    if (key.empty()) {
        m_root.appendValue(value);
    }
    else {
        auto &node = m_root.appendChild(m_factory, key);
        node.appendValue(value);
    }
}

template<typename K, typename V>
void RadixTree<K, V>::
clear()
{
    m_root.clear();
    m_factory.clear();
}

template<typename K, typename V>
size_t RadixTree<K, V>::
node_count() const
{
    size_t result = 0;

    m_root.traverse(
        [&result](NodeType const&, size_t) {
            ++result;
        }
    );

    return result;
}

template<typename K, typename V>
size_t RadixTree<K, V>::
value_count() const
{
    size_t result = 0;

    m_root.traverse(
        [&result](NodeType const& node, size_t) {
            if (node.hasValue()) {
                result += node.values().size();
            }
        }
    );

    return result;
}

template<typename K, typename V>
template<typename Visitor>
void RadixTree<K, V>::
traverse(Key const& query, Visitor &&visit) const
{
    if (m_root.hasValue()) {
        visit(m_root, m_root.key());
    }

    auto* node = &m_root;
    auto key = query;

    while (!key.empty()) {
        auto* const child = node->findPrefixChild(key);
        if (child) {
            auto const& childKey = child->key();

            if (visit(*child, childKey)) return;

            key = Key {
                next(key.begin(), childKey.size()),
                key.end()
            };
            node = child;
        }
        else {
            return;
        }
    }
}

template<typename K, typename V>
template<typename Visitor>
void RadixTree<K, V>::
traverse_all(Visitor &&visit) const
{
    m_root.traverse(visit);
}

} // namespace radix_tree
