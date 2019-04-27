#include "node.hpp"

#include "utility.hpp"

#include <boost/algorithm/string/predicate.hpp>

namespace radix_tree {

template<typename Key, typename Compare>
size_t
get_prefix_length(Key const& lhs, Key const& rhs, Compare const& is_equals)
{
    auto it1 = lhs.begin(), it2 = rhs.begin();
    auto const end1 = lhs.end(), end2 = rhs.end();
    size_t result = 0;

    for (; it1 != end1 && it2 != end2; ++result, ++it1, ++it2) {
        if (!is_equals(*it1, *it2)) break;
    }

    return result;
}

template<typename Key, typename Value, typename Traits>
node<Key, Value, Traits>::
node(Key const& key)
    : m_key { key }
{}

template<typename Key, typename Value, typename Traits>
node<Key, Value, Traits>::
~node()
{
    clear();
}

template<typename Key, typename Value, typename Traits>
typename node<Key, Value, Traits>::values_range_t node<Key, Value, Traits>::
values() const
{
    if (!m_values) {
        return {};
    }
    else {
        return *m_values;
    }
}

template<typename Key, typename Value, typename Traits>
node<Key, Value, Traits>* node<Key, Value, Traits>::
find_prefix_child(Key const& key) const
{
    namespace ba = boost::algorithm;

    if (!m_children) return nullptr;

    auto const it = find_partial_prefix_child(key);
    if (it != m_children->end() &&
                          ba::starts_with(key, it->key(), m_is_equals))
    {
        return &(*it);
    }
    else {
        return nullptr;
    }
}

template<typename Key, typename Value, typename Traits>
node<Key, Value, Traits>* node<Key, Value, Traits>::
find_prefix_child(Key const& key)
{
    return const_cast<node const*>(this)->find_prefix_child(key);
}

template<typename Key, typename Value, typename Traits>
bool node<Key, Value, Traits>::
has_child() const
{
    return static_cast<bool>(m_children);
}

template<typename Key, typename Value, typename Traits>
bool node<Key, Value, Traits>::
has_value() const
{
    return static_cast<bool>(m_values);
}

template<typename Key, typename Value, typename Traits>
size_t node<Key, Value, Traits>::
child_count() const
{
    return m_children ? m_children->size() : 0;
}

template<typename Key, typename Value, typename Traits>
template<typename Visitor>
void node<Key, Value, Traits>::
traverse(Visitor &&visit) const
{
    traverse(visit, 0);
}

template<typename Key, typename Value, typename Traits>
node<Key, Value, Traits>& node<Key, Value, Traits>::
append_child(node_factory<node>& factory, Key const& key)
{
    assert(!key.empty());

    if (!m_children) {
        m_children.reset(new children_t);
    }
    assert(m_children);

    auto it = find_partial_prefix_child(key);
    if (it == m_children->end()) {
        // Can't find prefix in children, so create a new child.
        auto &new_child = factory.new_node(key);
        auto const rv = m_children->insert(new_child);
        assert(rv.second); (void)rv;
        assert(&(*rv.first) == &new_child);
        return new_child;
    }
    else {
        auto const& child_key = it->key();
        auto& child = *it;

        auto const prefix_len = get_prefix_length(key, child_key, m_is_equals);
        assert(prefix_len > 0);

        // Duplicate key
        if (key.size() == child_key.size() && key.size() == prefix_len) {
            return child;
        }
        // Child's key is the prefix of the appending key
        // [before] [after appending "abcd"]
        //  abc      abc - b
        else if (prefix_len == child_key.size()) {
            Key const suffix { next(key.begin(), prefix_len), key.end() };
            return child.append_child(factory, suffix);
        }
        // Appending key is the prefix of the child's key
        // [before] [after appending "ab"]
        //  abc      ab - c
        else if (prefix_len == key.size()) {
            Key const suffix {
                next(child_key.begin(), prefix_len),
                child_key.end()
            };
            assert(!suffix.empty());

            auto &new_child = factory.new_node(key);

            auto num = m_children->erase(child);
            assert(num == 1); (void)num;
            child.m_key = suffix;

            new_child.m_children = std::make_unique<children_t>();
            auto rv = new_child.m_children->insert(child);
            assert(rv.second);

            rv = m_children->insert(new_child);
            assert(rv.second);
            assert(&(*rv.first) == &new_child);

            return new_child;
        }
        // Appending key and child's key have common prefix
        // [before] [after appending "abd"]
        //  abc      ab-+--c
        //              +--d
        else {
            assert(key.size() > prefix_len && child_key.size() > prefix_len);

            // split the child into a new branch and its child
            auto const suffix_it = next(child_key.begin(), prefix_len);
            Key const prefix { child_key.begin(), suffix_it },
                      suffix { suffix_it, child_key.end() };
            assert(!prefix.empty());
            assert(!suffix.empty());

            auto num = m_children->erase(child);
            assert(num == 1); (void)num;
            child.m_key = suffix;

            auto& new_branch = factory.new_node(prefix);
            new_branch.m_children = std::make_unique<children_t>();
            auto rv = new_branch.m_children->insert(child);
            assert(rv.second);
            assert(&(*rv.first) == &child);

            rv = m_children->insert(new_branch);
            assert(rv.second);
            assert(&(*rv.first) == &new_branch);

            // create a new node and append to the new branch
            Key const key_suffix { next(key.begin(), prefix_len), key.end() };
            auto& new_child = factory.new_node(key_suffix);
            rv = new_branch.m_children->insert(new_child);
            assert(rv.second);
            assert(&(*rv.first) == &new_child);

            return new_child;
        }
    }
}

template<typename Key, typename Value, typename Traits>
void node<Key, Value, Traits>::
append_value(Value const& value)
{
    if (!m_values) {
        m_values = std::make_unique<values_t>();
    }
    m_values->push_back(value);
}

template<typename Key, typename Value, typename Traits>
void node<Key, Value, Traits>::
clear()
{
    if (m_children) {
        for (auto& child: *m_children) {
            if (child.has_child()) {
                child.clear();
            }
        }

        m_children->clear();
        m_children.reset(nullptr);
    }

    m_values.reset(nullptr);
}

template<typename Key, typename Value, typename Traits>
template<typename Visitor>
void node<Key, Value, Traits>::
traverse(Visitor&& visit, size_t const level) const
{
    // Pre-order traversal algorithm
    visit(*this, level);
    if (!m_children) return;

    for (auto const& child: *m_children) {
        child.traverse(visit, level + 1);
    }
}

template<typename Key, typename Value, typename Traits>
typename node<Key, Value, Traits>::children_t::iterator node<Key, Value, Traits>::
find_partial_prefix_child(Key const& key) const
{
    assert(!key.empty());

    // Binary search by first charactor
    auto const it = m_children->lower_bound(key,
        [] (node const& node, Key const& key) {
            auto const& node_key = node.key();

            return m_char_compare(node_key.front(), key.front());
        }
    );

    if (it == m_children->end() ||
            !m_is_equals(it->key().front(), key.front()))
    {
        return m_children->end();
    }
    else {
        // There has to be only one prefix item.
        auto next = std::next(it);
        assert(!(next != m_children->end()
            && m_is_equals(next->key().front(), key.front()))); (void)next;

        return it;
    }
}

template<typename Key, typename Value, typename Traits>
typename node<Key, Value, Traits>::char_equals_t const node<Key, Value, Traits>::m_is_equals {};

template<typename Key, typename Value, typename Traits>
typename node<Key, Value, Traits>::char_compare_t const node<Key, Value, Traits>::m_char_compare {};

} // namespace radix_tree
