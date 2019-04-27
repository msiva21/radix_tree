#ifndef RADIX_TREE_TRAVERSE_HPP
#define RADIX_TREE_TRAVERSE_HPP

#include <map>
#include <string>

#include <boost/property_tree/ptree.hpp>

namespace radix_tree {

template<typename Key, typename Value>
boost::property_tree::ptree
statistics(RadixTree<Key, Value> const& tree)
{
    size_t numBranch = 0, numLeaf = 0;
    std::map<size_t, size_t> childCounts;
    std::map<size_t, size_t> valueCounts;
    std::map<size_t, size_t> branchsByLevel;
    std::map<size_t, size_t> leavesByLevel;

    tree.traverse_all(
        [&](auto const& node, auto const level) {
            if (node.hasChild()) {
                ++numBranch;
                ++childCounts[node.childCount()];
                ++branchsByLevel[level];
            }
            else {
                ++numLeaf;
                ++leavesByLevel[level];
            }
            ++valueCounts[node.values().size()];
        }
    );

    namespace bpt = boost::property_tree;
    bpt::ptree result;

    result.put("Number of leaf", numLeaf);
    result.put("Number of branch", numBranch);
    result.put("Number of nodes", tree.node_count());
    result.put("Number of values", tree.value_count());

    bpt::ptree child;
    for (auto const& item: childCounts) {
        child.put(std::to_string(item.first), item.second);
    }
    result.put_child("Branches by children", child);

    child.clear();
    for (auto const& item: branchsByLevel) {
        child.put(std::to_string(item.first), item.second);
    }
    result.put_child("Branches by level", child);

    child.clear();
    for (auto const& item: leavesByLevel) {
        child.put(std::to_string(item.first), item.second);
    }
    result.put_child("Leaves by level", child);

    child.clear();
    for (auto const& item: valueCounts) {
        child.put(std::to_string(item.first), item.second);
    }
    result.put_child("Nodes by values", child);

    return result;
}

} // namespace radix_tree

#endif // RADIX_TREE_TRAVERSE_HPP
