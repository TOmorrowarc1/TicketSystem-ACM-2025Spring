#ifndef TRAIN_SYS_HPP
#define TRAIN_SYS_HPP

#include "core_system.hpp"
#include "item/train.hpp"
#include <vector>
namespace train_sys {
extern int order_time;

extern bpt::BufferPoolManager state_buffer;
extern bpt::BPlusTree<TrainStateKey, TrainState, TrainStateComparator> states;

extern bpt::BufferPoolManager release_buffer;
extern bpt::BPlusTree<FixedString<20>, TrainTotal, FixStringComparator<20>>
    release;

extern bpt::BufferPoolManager routes_buffer;
extern bpt::BPlusTree<RouteTrain, RouteTrain, RouteTComparator> routes;

extern bpt::BufferPoolManager user_order_buffer;
extern bpt::BPlusTree<OrderKey, Order, OrderKeyComparator> user_order;
extern bpt::BufferPoolManager train_query_buffer;
extern bpt::BPlusTree<Query, Query, QueryComparator> train_query;

void AddTrain(const FixedString<20> &train_id, const TrainTotal &train);
void DeleteTrain(const FixedString<20> train_id);
void ReleaseTrain(const FixedString<20> train_id);
void QueryTrain(const FixedString<20> train_id, const Clock &time);
void QueryTicket(str_hash origin, str_hash des, const Clock date, bool time);
void QueryTransfer(str_hash origin, str_hash des, const Clock date, bool time);
void BuyTicket(Query &target, bool queue);
void QueryOrder(const FixedString<20> &uid);
void Refund(const FixedString<20> &uid, int rank = 0);

void RouteQuickSortT(RouteUser **target, int start, int end);
void RouteQuickSortP(RouteUser **target, int start, int end);
} // namespace train_sys

#endif