/*
Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.*/
#ifndef FHAMONIC_KNAPSACK_ALL_HPP
#define FHAMONIC_KNAPSACK_ALL_HPP

#ifndef FHAMONIC_KNAPSACK_BRANCH_AND_BOUND_HPP
#define FHAMONIC_KNAPSACK_BRANCH_AND_BOUND_HPP

#include <iterator>
#include <numeric>
#include <stack>
#include <type_traits>
#include <vector>

#include <range/v3/algorithm/remove_if.hpp>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/zip.hpp>

#ifndef FHAMONIC_KNAPSACK_INSTANCE_HPP
#define FHAMONIC_KNAPSACK_INSTANCE_HPP

#include <limits>
#include <vector>

namespace fhamonic {
namespace knapsack {

template <typename Value, typename Cost>
class Instance {
public:
    class Item {
    public:
        Value value;
        Cost cost;

    public:
        Item(Value v, Cost c) : value{v}, cost{c} {}
        Item(const Item & item) : value{item.value}, cost{item.cost} {}
        double getRatio() const {
            if(cost == 0) return std::numeric_limits<double>::max();
            return static_cast<double>(value) / static_cast<double>(cost);
        }
        bool operator<(const Item & other) const {
            return getRatio() > other.getRatio();
        }
    };

private:
    Cost budget;
    std::vector<Item> items;

public:
    Instance() {}

    void setBudget(Cost b) { budget = b; }
    Cost getBudget() const { return budget; }

    void addItem(Value v, Cost w) { items.push_back(Item(v, w)); }
    size_t itemCount() const { return items.size(); }

    const std::vector<Item> & getItems() const { return items; }
    const Item getItem(const std::size_t i) const { return items[static_cast<std::size_t>(i)]; }
    const Item operator[](const std::size_t i) const { return items[i]; }
};

}  // namespace Knapsack
}  // namespace fhamonic

#endif  // FHAMONIC_KNAPSACK_INSTANCE_HPP
#ifndef FHAMONIC_KNAPSACK_SOLUTION_HPP
#define FHAMONIC_KNAPSACK_SOLUTION_HPP

#include <vector>

namespace fhamonic {
namespace knapsack {

template <typename Value, typename Cost>
class Solution {
private:
    const Instance<Value, Cost> & instance;
    std::vector<bool> _taken;

public:
    Solution(const Instance<Value, Cost> & i)
        : instance(i), _taken(i.itemCount()) {}

    void add(const std::size_t i) { _taken[i] = true; }
    void set(const std::size_t i, bool b) { _taken[i] = b; }
    void remove(const std::size_t i) { _taken[i] = false; }
    bool isTaken(const std::size_t i) { return _taken[static_cast<std::size_t>(i)]; }

    auto & operator[](const std::size_t i) { return _taken[i]; }

    Value getValue() const {
        Value sum{};
        for(std::size_t i = 0; i < instance.itemCount(); ++i)
            if(_taken[i]) sum += instance[i].value;
        return sum;
    }
    Cost getCost() const {
        Cost sum{};
        for(std::size_t i = 0; i < instance.itemCount(); ++i)
            if(_taken[i]) sum += instance[i].cost;
        return sum;
    }
};

}  // namespace knapsack
}  // namespace fhamonic

#endif  // FHAMONIC_KNAPSACK_SOLUTION_HPP

namespace fhamonic {
namespace knapsack {

template <typename Value, typename Cost>
class BranchAndBound {
public:
    using TInstance = Instance<Value, Cost>;
    using TItem = typename TInstance::Item;
    using TSolution = Solution<Value, Cost>;

private:
    //     double computeUpperBound(const std::vector<TItem> & sorted_items,
    //                          std::size_t depth, Value bound_value,
    //                          Cost bound_budget_left) {
    //     for(; depth < sorted_items.size(); ++depth) {
    //         const TItem & item = sorted_items[depth];
    //         if(bound_budget_left < item.cost)
    //             return bound_value + bound_budget_left * item.getRatio();
    //         bound_budget_left -= item.cost;
    //         bound_value += item.value;
    //     }
    //     return bound_value;
    // }

    // std::vector<std::size_t> iterative_bnb(
    //     const std::vector<TItem> & sorted_items, Cost budget_left) {
    //     const std::size_t nb_items = sorted_items.size();
    //     std::size_t depth = 0;
    //     Value value = 0;
    //     Value best_value = 0;
    //     std::vector<std::size_t> stack;
    //     std::vector<std::size_t> best_stack;
    //     goto begin;
    // backtrack:
    //     while(!stack.empty()) {
    //         depth = stack.back();
    //         stack.pop_back();
    //         value -= sorted_items[depth].value;
    //         budget_left += sorted_items[depth].cost;
    //         for(++depth; depth < nb_items; ++depth) {
    //             if(budget_left < sorted_items[depth].cost) continue;
    //             if(computeUpperBound(sorted_items, depth, value, budget_left)
    //             <=
    //                best_value)
    //                 goto backtrack;
    //         begin:
    //             value += sorted_items[depth].value;
    //             budget_left -= sorted_items[depth].cost;
    //             stack.push_back(depth);
    //         }
    //         if(value <= best_value) continue;
    //         best_value = value;
    //         best_stack = stack;
    //     }
    //     return best_stack;
    // }

    double computeUpperBound(auto it, const auto end, Value bound_value,
                             Cost bound_budget_left) {
        for(; it < end; ++it) {
            if(bound_budget_left < it->cost)
                return bound_value + bound_budget_left * it->value / it->cost;
            bound_budget_left -= it->cost;
            bound_value += it->value;
        }

        return bound_value;
    }

    auto iterative_bnb(const std::vector<TItem> & sorted_items,
                       Cost budget_left) {
        Value value = 0;
        Value best_value = 0;
        auto it = sorted_items.cbegin();
        const auto end = sorted_items.cend();
        std::vector<decltype(it)> stack;
        std::vector<decltype(it)> best_stack;
        goto begin;
    backtrack:
        while(!stack.empty()) {
            it = stack.back();
            stack.pop_back();
            value -= it->value;
            budget_left += it->cost;
            for(++it; it < end; ++it) {
                if(budget_left < it->cost) continue;
                if(computeUpperBound(it, end, value, budget_left) <= best_value)
                    goto backtrack;
            begin:
                value += it->value;
                budget_left -= it->cost;
                stack.push_back(it);
            }
            if(value <= best_value) continue;
            best_value = value;
            best_stack = stack;
        }
        return best_stack;
    }

public:
    BranchAndBound() {}

    TSolution solve(const TInstance & instance) {
        TSolution solution(instance);
        if(instance.itemCount() > 0) {
            std::vector<TItem> sorted_items = instance.getItems();
            std::vector<std::size_t> permuted_id(instance.itemCount());
            std::iota(permuted_id.begin(), permuted_id.end(), 0);

            auto zip_view = ranges::view::zip(sorted_items, permuted_id);
            auto end = ranges::remove_if(zip_view, [&](const auto & r) {
                return r.first.cost > instance.getBudget();
            });
            const ptrdiff_t new_size = std::distance(zip_view.begin(), end);
            sorted_items.erase(sorted_items.begin() + new_size,
                               sorted_items.end());
            permuted_id.erase(permuted_id.begin() + new_size,
                              permuted_id.end());
            ranges::sort(zip_view,
                         [](auto p1, auto p2) { return p1.first < p2.first; });

            // auto best_stack =
            //     iterative_bnb(sorted_items, instance.getBudget());
            // for(std::size_t i : best_stack) {
            //     solution.add(permuted_id[i]);
            // }

            auto best_stack = iterative_bnb(sorted_items, instance.getBudget());
            for(auto && it : best_stack) {
                solution.add(permuted_id[static_cast<std::size_t>(
                    std::distance(sorted_items.cbegin(), it))]);
            }
        }
        return solution;
    }
};
}  // namespace knapsack
}  // namespace fhamonic

#endif  // FHAMONIC_KNAPSACK_BRANCH_AND_BOUND_HPP
#ifndef FHAMONIC_KNAPSACK_DYNAMIC_PROGRAMMING_HPP
#define FHAMONIC_KNAPSACK_DYNAMIC_PROGRAMMING_HPP

#include <algorithm>
#include <numeric>
#include <type_traits>

#include <range/v3/view/reverse.hpp>

namespace fhamonic {
namespace knapsack {

template <
    typename Value, typename Cost,
    class = typename std::enable_if<std::is_integral<Cost>::value, bool>::type>
class DynamicProgramming {
public:
    using TInstance = Instance<Value, Cost>;
    using TItem = typename TInstance::Item;
    using TSolution = Solution<Value, Cost>;

public:
    DynamicProgramming() {}

    TSolution solve(const TInstance & instance) {
        const std::size_t nb_items = instance.itemCount();
        const Cost budget = instance.getBudget();

        auto tab = std::make_unique<Value[]>(
            (nb_items + 1) * static_cast<std::size_t>(budget + 1));

        Value * previous_tab = tab.get();
        for(Cost w = 0; w < budget; ++w) {
            previous_tab[w] = 0;
        }

        for(const auto & item : instance.getItems()) {
            Value * const current_tab = previous_tab + budget + 1;
            Cost w = std::min(budget, item.cost);
            std::copy(previous_tab, previous_tab + w, current_tab);
            for(; w <= budget; ++w) {
                current_tab[w] = std::max(
                    previous_tab[w], previous_tab[w - item.cost] + item.value);
            }
            previous_tab = current_tab;
        }

        TSolution solution(instance);
        const Value * step = previous_tab + budget;
        for(std::size_t i = (nb_items - 1); i > 0; --i) {
            const bool taken = (*step > *(step - budget - 1));
            solution.set(i, taken);
            step -= budget + 1 + taken * instance[i].cost;
        }
        const bool taken = (*step > *(step - budget - 1));
        solution.set(0, taken);

        return solution;
    }
};

}  // namespace knapsack
}  // namespace fhamonic

#endif  // FHAMONIC_KNAPSACK_DYNAMIC_PROGRAMMING_HPP
#ifndef UBOUNDED_FHAMONIC_KNAPSACK_BRANCH_AND_BOUND_HPP
#define UBOUNDED_FHAMONIC_KNAPSACK_BRANCH_AND_BOUND_HPP

#include <numeric>
#include <stack>
#include <vector>

#include <range/v3/algorithm/remove_if.hpp>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/zip.hpp>

#ifndef UNBOUNDED_FHAMONIC_KNAPSACK_INSTANCE_HPP
#define UNBOUNDED_FHAMONIC_KNAPSACK_INSTANCE_HPP

#include <limits>
#include <vector>

namespace fhamonic {
namespace unbounded_knapsack {

template <typename Value, typename Cost>
using Instance = Instance<Value, Cost>;

}  // namespace unbounded_knapsack
}  // namespace fhamonic

#endif  // UNBOUNDED_FHAMONIC_KNAPSACK_INSTANCE_HPP
#ifndef UNBOUNDED_FHAMONIC_KNAPSACK_SOLUTION_HPP
#define UNBOUNDED_FHAMONIC_KNAPSACK_SOLUTION_HPP

#include <vector>

namespace fhamonic {
namespace unbounded_knapsack {

template <typename Value, typename Cost>
class Solution {
private:
    const Instance<Value, Cost> & instance;
    std::vector<int> _nb_taken;

public:
    Solution(const Instance<Value, Cost> & i)
        : instance(i), _nb_taken(i.itemCount()) {}

    void add(size_t i) { ++_nb_taken[i]; }
    void set(size_t i, int n) { _nb_taken[i] = n; }
    void remove(size_t i) { _nb_taken[i] = 0; }
    bool isTaken(size_t i) { return _nb_taken[i] > 0; }

    int & operator[](size_t i) { return _nb_taken[i]; }

    Value getValue() const {
        Value sum{};
        for(size_t i = 0; i < instance.itemCount(); ++i)
            sum += _nb_taken[i] * instance[i].value;
        return sum;
    }
    Cost getCost() const {
        Cost sum{};
        for(size_t i = 0; i < instance.itemCount(); ++i)
            sum += _nb_taken[i] * instance[i].cost;
        return sum;
    }
};

}  // namespace unbounded_knapsack
}  // namespace fhamonic

#endif  // UNBOUNDED_FHAMONIC_KNAPSACK_SOLUTION_HPP

namespace fhamonic {
namespace unbounded_knapsack {

template <typename Value, typename Cost>
class BranchAndBound {
public:
    using TInstance = Instance<Value, Cost>;
    using TItem = typename TInstance::Item;
    using TSolution = Solution<Value, Cost>;

private:
    double computeUpperBound(const std::vector<TItem> & sorted_items,
                             std::size_t depth, Value bound_value,
                             Cost bound_budget_left) {
        for(;depth < sorted_items.size();++depth) {
            const TItem & item = sorted_items[depth];
        // for(const TItem & item : ranges::views::drop(sorted_items, depth)) {
            if(bound_budget_left <= item.cost)
                return bound_value + bound_budget_left * item.getRatio();
            const int nb_take = bound_budget_left / item.cost;
            bound_budget_left -= nb_take * item.cost;
            bound_value += nb_take * item.value;
        }
        return bound_value;
    }

    std::stack<std::pair<std::size_t, int>> iterative_bnb(
        const std::vector<TItem> & sorted_items, Cost budget_left) {
        const std::size_t nb_items = sorted_items.size();
        std::size_t depth = 0;
        Value value = 0;
        Value best_value = 0;
        std::stack<std::pair<std::size_t, int>> stack;
        std::stack<std::pair<std::size_t, int>> best_stack;
        goto begin;
    backtrack:
        while(!stack.empty()) {
            depth = stack.top().first;
            if(--stack.top().second == 0) stack.pop();
            value -= sorted_items[depth].value;
            budget_left += sorted_items[depth++].cost;
            for(; depth < nb_items; ++depth) {
                if(budget_left < sorted_items[depth].cost) continue;
                if(computeUpperBound(sorted_items, depth, value, budget_left) <=
                   best_value)
                    goto backtrack;
            begin:
                const int nb_take = budget_left / sorted_items[depth].cost;
                value += nb_take * sorted_items[depth].value;
                budget_left -= nb_take * sorted_items[depth].cost;
                stack.emplace(depth, nb_take);
            }
            if(value <= best_value) continue;
            best_value = value;
            best_stack = stack;
        }
        return best_stack;
    }

public:
    BranchAndBound() {}

    TSolution solve(const TInstance & instance) {
        std::vector<TItem> sorted_items = instance.getItems();
        std::vector<std::size_t> permuted_id(instance.itemCount());
        std::iota(permuted_id.begin(), permuted_id.end(), 0);

        auto zip_view = ranges::view::zip(sorted_items, permuted_id);
        auto end = ranges::remove_if(zip_view, [&](const auto & r) {
            return r.first.cost > instance.getBudget();
        });
        const ptrdiff_t new_size = std::distance(zip_view.begin(), end);
        sorted_items.erase(sorted_items.begin() + new_size, sorted_items.end());
        permuted_id.erase(permuted_id.begin() + new_size, permuted_id.end());
        ranges::sort(zip_view,
                     [](auto p1, auto p2) { return p1.first < p2.first; });

        std::stack<std::pair<std::size_t, int>> best_stack =
            iterative_bnb(sorted_items, instance.getBudget());

        TSolution solution(instance);
        while(!best_stack.empty()) {
            solution.set(permuted_id[best_stack.top().first],
                         best_stack.top().second);
            best_stack.pop();
        }
        return solution;
    }
};

}  // namespace unbounded_knapsack
}  // namespace fhamonic

#endif  // UBOUNDED_FHAMONIC_KNAPSACK_BRANCH_AND_BOUND_HPP

#endif  // FHAMONIC_KNAPSACK_ALL_HPP