#ifndef RADIX_TREE_TRAVERSE_HPP
#define RADIX_TREE_TRAVERSE_HPP

#include <map>
#include <string>

#include <boost/property_tree/ptree.hpp>

namespace radix_tree {

template<typename Key, typename Value>
boost::property_tree::ptree
statistics(tree<Key, Value> const& tree)
{
    size_t num_branch = 0, num_leaf = 0;
    std::map<size_t, size_t> child_counts;
    std::map<size_t, size_t> value_counts;
    std::map<size_t, size_t> branchs_by_level;
    std::map<size_t, size_t> leaves_by_level;

    tree.traverse_all(
        [&](auto const& node, auto const level) {
            if (node.has_child()) {
                ++num_branch;
                ++child_counts[node.child_count()];
                ++branchs_by_level[level];
            }
            else {
                ++num_leaf;
                ++leaves_by_level[level];
            }
            ++value_counts[node.values().size()];
        }
    );

    namespace bpt = boost::property_tree;
    bpt::ptree result;

    result.put("Number of leaf", num_leaf);
    result.put("Number of branch", num_branch);
    result.put("Number of nodes", tree.node_count());
    result.put("Number of values", tree.value_count());

    bpt::ptree child;
    for (auto const& item: child_counts) {
        child.put(std::to_string(item.first), item.second);
    }
    result.put_child("Branches by children", child);

    child.clear();
    for (auto const& item: branchs_by_level) {
        child.put(std::to_string(item.first), item.second);
    }
    result.put_child("Branches by level", child);

    child.clear();
    for (auto const& item: leaves_by_level) {
        child.put(std::to_string(item.first), item.second);
    }
    result.put_child("Leaves by level", child);

    child.clear();
    for (auto const& item: value_counts) {
        child.put(std::to_string(item.first), item.second);
    }
    result.put_child("Nodes by values", child);

    return result;
}

} // namespace radix_tree

#endif // RADIX_TREE_TRAVERSE_HPP
