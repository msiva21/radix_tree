#ifndef RADIX_TREE_NODE_HPP
#define RADIX_TREE_NODE_HPP

#include "node_factory.hpp"

#include <iterator>
#include <locale>
#include <memory>
#include <vector>

#include <boost/range/iterator_range.hpp>
#include <boost/intrusive/set.hpp>

namespace radix_tree {

template<typename Key>
struct case_insensitive_node_traits
{
    using char_t = typename Key::value_type;

    struct char_equals {
        bool operator()(char_t const lhs, char_t const rhs) const {
            static auto const& ct =
                std::use_facet<std::ctype<char_t>>(std::locale {});
            return ct.toupper(lhs) == ct.toupper(rhs);
        }
    };

    struct char_compare {
        bool operator()(char_t const lhs, char_t const rhs) const {
            static auto const& ct =
                std::use_facet<std::ctype<char_t>>(std::locale {});
            return ct.toupper(lhs) < ct.toupper(rhs);
        }
    };
};

namespace bi = boost::intrusive;

template<typename Key, typename Value, typename Traits = case_insensitive_node_traits<Key>>
class node : public bi::set_base_hook<bi::optimize_size<true>>
{
public:
    using key_t = Key;
    using value_t = Value;
    using char_equals_t = typename Traits::char_equals;
    using char_compare_t = typename Traits::char_compare;

    struct child_compare {
        bool operator()(node const& lhs, node const& rhs) const {
            static char_compare_t const compare {};
            return compare(lhs.key().front(), rhs.key().front());
        }
    };
    using children_t = bi::set<node, bi::compare<child_compare>>;
    using values_t = std::vector<Value>;
    using values_range_t =
        boost::iterator_range<typename values_t::const_iterator>;
public:
    // constructor / destructor
    explicit node(Key const&);
    node() = default;
    ~node();

    // accessor
    Key const& key() const { return m_key; }

    values_range_t values() const;

    // query
    node* find_prefix_child(Key const&) const;
    node* find_prefix_child(Key const&);

    bool has_child() const;
    bool has_value() const;
    size_t child_count() const;

    template<typename Visitor>
        void traverse(Visitor&&) const;

    void validate() const
    {
        assert(!m_key.empty());

        if (m_children) {
            auto curr = m_children->begin(), next = std::next(curr);
            auto const end = m_children->end();
            for (; next != end; ++curr, ++next) {
                assert(!m_is_equals(curr->key().front(), next->key().front()));
            }
        }
    }

    // modifier
    node& append_child(node_factory<node>&, Key const&);
    void append_value(Value const&);

    void clear();

private:
    template<typename Visitor>
        void traverse(Visitor&&, size_t const level) const;

    typename children_t::iterator
        find_partial_prefix_child(Key const&) const;

private:
    Key m_key;
    std::unique_ptr<children_t> m_children;
    std::unique_ptr<values_t> m_values;

    static const char_equals_t m_is_equals;
    static const char_compare_t m_char_compare;
};

} // namespace radix_tree

#include "node.tcc"

#endif // RADIX_TREE_NODE_HPP
