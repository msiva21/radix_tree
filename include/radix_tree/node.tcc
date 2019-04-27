#include "node.hpp"

#include "utility.hpp"

#include <boost/algorithm/string/predicate.hpp>

namespace radix_tree {

template<typename K, typename V, typename T>
Node<K, V, T>::
Node(Key const& key)
    : m_key { key }
{}

template<typename K, typename V, typename T>
Node<K, V, T>::
~Node()
{
    clear();
}

template<typename K, typename V, typename T>
typename Node<K, V, T>::ValuesRange Node<K, V, T>::
values() const
{
    if (!m_values) {
        return {};
    }
    else {
        return *m_values;
    }
}

template<typename K, typename V, typename T>
Node<K, V, T>* Node<K, V, T>::
findPrefixChild(Key const& key) const
{
    namespace ba = boost::algorithm;

    if (!m_children) return nullptr;

    auto const it = findPartialPrefixChild(key);
    if (it != m_children->end() &&
                          ba::starts_with(key, it->key(), m_isEquals))
    {
        return &(*it);
    }
    else {
        return nullptr;
    }
}

template<typename K, typename V, typename T>
Node<K, V, T>* Node<K, V, T>::
findPrefixChild(Key const& key)
{
    return const_cast<Node const*>(this)->findChild(key);
}

template<typename K, typename V, typename T>
bool Node<K, V, T>::
hasChild() const
{
    return static_cast<bool>(m_children);
}

template<typename K, typename V, typename T>
bool Node<K, V, T>::
hasValue() const
{
    return static_cast<bool>(m_values);
}

template<typename K, typename V, typename T>
size_t Node<K, V, T>::
childCount() const
{
    return m_children ? m_children->size() : 0;
}

template<typename K, typename V, typename T>
template<typename Visitor>
void Node<K, V, T>::
traverse(Visitor &&visit) const
{
    traverse(visit, 0);
}

template<typename Key, typename Compare>
size_t
getPrefixLength(Key const& lhs, Key const& rhs, Compare const& isEquals)
{
    auto it1 = lhs.begin(), it2 = rhs.begin();
    auto const end1 = lhs.end(), end2 = rhs.end();
    size_t result = 0;

    for (; it1 != end1 && it2 != end2; ++result, ++it1, ++it2) {
        if (!isEquals(*it1, *it2)) break;
    }

    return result;
}

template<typename K, typename V, typename T>
Node<K, V, T>& Node<K, V, T>::
appendChild(NodeFactory<Node>& factory, Key const& key)
{
    assert(!key.empty());

    if (!m_children) {
        m_children.reset(new Children);
    }
    assert(m_children);

    auto it = findPartialPrefixChild(key);
    if (it == m_children->end()) {
        // Can't find prefix in children, so create a new child.
        auto &newChild = factory.newNode(key);
        auto const rv = m_children->insert(newChild);
        assert(rv.second); (void)rv;
        assert(&(*rv.first) == &newChild);
        return newChild;
    }
    else {
        auto const& childKey = it->key();
        auto& child = *it;

        auto const prefixLen = getPrefixLength(key, childKey, m_isEquals);
        assert(prefixLen > 0);

        // Duplicate key
        if (key.size() == childKey.size() && key.size() == prefixLen) {
            return child;
        }
        // Child's key is the prefix of the appending key
        // [before] [after appending "abcd"]
        //  abc      abc - b
        else if (prefixLen == childKey.size()) {
            Key const suffix { next(key.begin(), prefixLen), key.end() };
            return child.appendChild(factory, suffix);
        }
        // Appending key is the prefix of the child's key
        // [before] [after appending "ab"]
        //  abc      ab - c
        else if (prefixLen == key.size()) {
            Key const suffix {
                next(childKey.begin(), prefixLen),
                childKey.end()
            };
            assert(!suffix.empty());

            auto &newChild = factory.newNode(key);

            auto num = m_children->erase(child);
            assert(num == 1); (void)num;
            child.m_key = suffix;

            newChild.m_children = std::make_unique<Children>();
            auto rv = newChild.m_children->insert(child);
            assert(rv.second);

            rv = m_children->insert(newChild);
            assert(rv.second);
            assert(&(*rv.first) == &newChild);

            return newChild;
        }
        // Appending key and child's key have common prefix
        // [before] [after appending "abd"]
        //  abc      ab-+--c
        //              +--d
        else {
            assert(key.size() > prefixLen && childKey.size() > prefixLen);

            // split the child into a new branch and its child
            auto const suffixIt = next(childKey.begin(), prefixLen);
            Key const prefix { childKey.begin(), suffixIt },
                      suffix { suffixIt, childKey.end() };
            assert(!prefix.empty());
            assert(!suffix.empty());

            auto num = m_children->erase(child);
            assert(num == 1); (void)num;
            child.m_key = suffix;

            auto& newBranch = factory.newNode(prefix);
            newBranch.m_children = std::make_unique<Children>();
            auto rv = newBranch.m_children->insert(child);
            assert(rv.second);
            assert(&(*rv.first) == &child);

            rv = m_children->insert(newBranch);
            assert(rv.second);
            assert(&(*rv.first) == &newBranch);

            // create a new node and append to the new branch
            Key const keySuffix { next(key.begin(), prefixLen), key.end() };
            auto& newChild = factory.newNode(keySuffix);
            rv = newBranch.m_children->insert(newChild);
            assert(rv.second);
            assert(&(*rv.first) == &newChild);

            return newChild;
        }
    }
}

template<typename K, typename V, typename T>
void Node<K, V, T>::
appendValue(Value const& value)
{
    if (!m_values) {
        m_values = std::make_unique<Values>();
    }
    m_values->push_back(value);
}

template<typename K, typename V, typename T>
inline void Node<K, V, T>::
clear()
{
    if (m_children) {
        for (auto& child: *m_children) {
            if (child.hasChild()) {
                child.clear();
            }
        }

        m_children->clear();
        m_children.reset(nullptr);
    }

    m_values.reset(nullptr);
}

template<typename K, typename V, typename T>
template<typename Visitor>
void Node<K, V, T>::
traverse(Visitor&& visit, size_t const level) const
{
    // Pre-order traversal algorithm
    visit(*this, level);
    if (!m_children) return;

    for (auto const& child: *m_children) {
        child.traverse(visit, level + 1);
    }
}

template<typename K, typename V, typename T>
typename Node<K, V, T>::Children::iterator Node<K, V, T>::
findPartialPrefixChild(Key const& key) const
{
    assert(!key.empty());

    // Binary search by first charactor
    auto const it = m_children->lower_bound(key,
        [] (Node const& node, Key const& key) {
            auto const& nodeKey = node.key();

            return m_charCompare(nodeKey.front(), key.front());
        }
    );

    if (it == m_children->end() ||
            !m_isEquals(it->key().front(), key.front()))
    {
        return m_children->end();
    }
    else {
        // There has to be only one prefix item.
        auto next = std::next(it);
        assert(!(next != m_children->end()
            && m_isEquals(next->key().front(), key.front()))); (void)next;

        return it;
    }
}

template<typename K, typename V, typename T>
typename Node<K, V, T>::CharEquals const Node<K, V, T>::m_isEquals {};

template<typename K, typename V, typename T>
typename Node<K, V, T>::CharCompare const Node<K, V, T>::m_charCompare {};

} // namespace radix_tree
