#ifndef TRAIN_SYS_HPP
#define TRAIN_SYS_HPP

#include "core_system.hpp"
#include "item/train.hpp"
#include <vector>
namespace train_sys {
extern bpt::BufferPoolManager state_buffer;
extern bpt::BPlusTree<TrainStateKey, TrainState, TrainStateComparator> states;

extern bpt::BufferPoolManager release_buffer;
extern bpt::BPlusTree<FixedString<20>, TrainTotal, FixStringComparator<20>>
    release;

extern bpt::BufferPoolManager routeA_buffer;
extern bpt::BPlusTree<RouteTrain, RouteTrain, RouteTComparatorA> routeA;
extern bpt::BufferPoolManager routeB_buffer;
extern bpt::BPlusTree<RouteTrain, RouteTrain, RouteTComparatorB> routeB;

extern bpt::BufferPoolManager user_order_buffer;
extern bpt::BPlusTree<Order, Order, OrderUserComparator> user_order;
extern bpt::BufferPoolManager train_order_buffer;
extern bpt::BPlusTree<Order, Order, OrderTrainComparator> train_order;

auto AddTrain(const FixedString<20> &train_id, const TrainTotal &train) -> bool;
auto DeleteTrain(const FixedString<20> train_id) -> bool;
auto ReleaseTrain(const FixedString<20> train_id) -> bool;
auto QueryTrain(const FixedString<20>, const Clock &time) -> bool;
void QueryTicket(const FixedChineseString<10> &start,
                 const FixedChineseString<10> &end, const Clock date,
                 bool time);
void QueryTransfer(const FixedChineseString<10> &start,
                   const FixedChineseString<10> &end, const Clock date,
                   bool time);
void BuyTicket(Order &target, bool queue);
auto QueryOrder(const FixedString<20> &uid) -> bool;
auto Refund(const FixedString<20> &uid, int rank = 0) -> bool;
} // namespace train_sys

#endif