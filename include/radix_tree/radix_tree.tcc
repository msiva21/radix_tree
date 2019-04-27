#include "radix_tree.hpp"

#include "utility.hpp"

namespace radix_tree {

template<typename Key, typename Value>
void tree<Key, Value>::
insert(Key const& key, Value const& value)
{
    if (key.empty()) {
        m_root.append_value(value);
    }
    else {
        auto &node = m_root.append_child(m_factory, key);
        node.append_value(value);
    }
}

template<typename Key, typename Value>
void tree<Key, Value>::
clear()
{
    m_root.clear();
    m_factory.clear();
}

template<typename Key, typename Value>
size_t tree<Key, Value>::
node_count() const
{
    size_t result = 0;

    m_root.traverse(
        [&](auto&, auto) { ++result; }
    );

    return result;
}

template<typename Key, typename Value>
size_t tree<Key, Value>::
value_count() const
{
    size_t result = 0;

    m_root.traverse(
        [&](auto& node, auto) {
            if (node.has_value()) {
                result += node.values().size();
            }
        }
    );

    return result;
}

template<typename Key, typename Value>
template<typename Visitor>
void tree<Key, Value>::
traverse(Key const& query, Visitor &&visit) const
{
    if (m_root.has_value()) {
        visit(m_root, m_root.key());
    }

    auto* node = &m_root;
    auto key = query;

    while (!key.empty()) {
        auto* const child = node->find_prefix_child(key);
        if (child) {
            auto const& child_key = child->key();

            if (visit(*child, child_key)) return;

            key = Key {
                next(key.begin(), child_key.size()),
                key.end()
            };
            node = child;
        }
        else {
            return;
        }
    }
}

template<typename Key, typename Value>
template<typename Visitor>
void tree<Key, Value>::
traverse_all(Visitor &&visit) const
{
    m_root.traverse(visit);
}

} // namespace radix_tree
