#ifndef B_PLUS_TREE_HPP
#define B_PLUS_TREE_HPP

#include "b_plus_tree_page.hpp"
#include "src/storage/buffer_pool_manager.hpp"
namespace bpt {

#define BPT_TYPE BPlusTree<KeyType, ValueType, KeyComparator>
struct Trace {
  PageGuard pages_trace[10];
  int place_trace[10] = {0};
  int levels = 0;
};

struct HeaderPage {
  int root_page_id_ = INVALID_PAGE_ID;
};

TEMPLATE
class BPlusTree {
  using InternalPage = InternalPage<KeyType, page_id_t, KeyComparator>;
  using LeafPage = LeafPage<KeyType, ValueType, KeyComparator>;

private:
  BufferPoolManager *bpm_;
  int leaf_max_size_;
  int internal_max_size_;
  int leaf_min_size_;
  int internal_min_size_;
  page_id_t header_page_id_;

  void FindPath(Trace &trace, const KeyType &key) const;
  auto SplitLeafNode(LeafPage *cursor) -> std::pair<KeyType, page_id_t>;
  auto SplitInternalNode(InternalPage *cursor) -> std::pair<KeyType, page_id_t>;

  auto SelectSibling(InternalPage *parent, const KeyType &key,
                     PageGuard &result) -> bool;

public:
  explicit BPlusTree(page_id_t header_page_id,
                     BufferPoolManager *buffer_pool_manager,
                     int leaf_max_size = PAGE_MAX_SIZE,
                     int internal_max_size = PAGE_MAX_SIZE);

  auto GetValue(const KeyType &key, std::vector<ValueType> *result) -> bool;
  auto Insert(const KeyType &key, const ValueType &value) -> bool;
  void Remove(const KeyType &key);
};

TEMPLATE
BPT_TYPE::BPlusTree(page_id_t header_page_id,
                    BufferPoolManager *buffer_pool_manager,
                    int leaf_max_size = PAGE_MAX_SIZE,
                    int internal_max_size = PAGE_MAX_SIZE - 1)
    : bpm_(buffer_pool_manager), leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size + 1),
      header_page_id_(header_page_id) {
  PageGuard guard = bpm_->VisitPage(header_page_id_, false);
  auto head_page = guard.AsMut<HeaderPage>();
  head_page->root_page_id_ = INVALID_PAGE_ID;
  leaf_min_size_ = leaf_max_size_ >> 1;
  internal_min_size_ = internal_max_size_ >> 1;
}

TEMPLATE
auto BPT_TYPE::GetValue(const KeyType &key, std::vector<ValueType> *result)
    -> bool {
  // Declaration of context instance.
  PageGuard read_guard = bpm_->VisitPage(header_page_id_, true);
  page_id_t root_id = read_guard.As<HeaderPage>()->root_page_id_;
  if (root_id == INVALID_PAGE_ID) {
    return false;
  }
  read_guard = bpm_->VisitPage(root_id, true);
  auto cursor_pointer = read_guard.As<InternalPage>();
  while (!cursor_pointer->IsLeafPage()) {
    read_guard = bpm_->VisitPage(
        cursor_pointer->ValueAt(cursor_pointer->KeyIndex(key) - 1));
    cursor_pointer = read_guard.As<InternalPage>();
  }
  auto cursor_leaf_pointer = read_guard.As<LeafPage>();
  int cursor = cursor_leaf_pointer->KeyIndex(key);
  if (cursor >= cursor_leaf_pointer->GetSize() ||
      KeyComparator(cursor_leaf_pointer->KeyAt(cursor), key) != 0) {
    return false;
  }
  result->push_back(cursor_leaf_pointer->ValueAt(cursor));
  return true;
}

TEMPLATE
void BPT_TYPE::FindPath(Trace &trace, const KeyType &key) const {
  page_id_t cursor = 0;
  PageGuard page_guard;
  cursor = trace.place_trace[0];
  page_guard = bpm_->VisitPage(cursor, false);
  auto cursor_pointer = page_guard.As<InternalPage>();
  trace.pages_trace[trace.levels] = std::move(page_guard);
  while (!cursor_pointer->IsLeafPage()) {
    cursor = cursor_pointer->KeyIndex(key) - 1;
    trace.place_trace[trace.levels] = cursor;
    ++trace.level_;
    page_guard = bpm_->VisitPage(cursor_pointer->ValueAt(cursor), false);
    cursor_pointer = page_guard.As<InternalPage>();
    trace.pages_trace[trace.levels] = std::move(page_guard);
  }
}

TEMPLATE
auto BPT_TYPE::SplitLeafNode(LeafPage *cursor)
    -> std::pair<KeyType, page_id_t> {
  page_id_t new_leaf = bpm_->NewPage();
  PageGuard page_guard = bpm_->VisitPage(new_leaf, false);
  auto cursor_new_leaf = page_guard.AsMut<LeafPage>();
  KeyType middle_key = cursor->SplitPage(cursor_new_leaf);
  cursor_new_leaf->SetNextPageId(cursor->GetNextPageId());
  cursor->SetNextPageId(new_leaf);
  return {middle_key, new_leaf};
}
TEMPLATE
auto BPT_TYPE::SplitInternalNode(InternalPage *cursor)
    -> std::pair<KeyType, page_id_t> {
  page_id_t new_internal = bpm_->NewPage();
  PageGuard page_guard = bpm_->VisitPage(new_internal, false);
  auto cursor_new_internal = page_guard.AsMut<InternalPage>();
  return {cursor->SplitPage(cursor_new_internal), new_internal};
}

TEMPLATE
auto BPT_TYPE::SelectSibling(InternalPage *parent, const KeyType &key,
                             PageGuard &result) -> bool {
  int location = parent->KeyIndex(key) - 1;
  bool result_right = false;
  if (location == 0) {
    result = bpm_->VisitPage(parent->ValueAt(1), false);
    result_right = true;
  } else if (location == parent->GetSize() - 1) {
    result = bpm_->VisitPage(parent->ValueAt(location - 1), false);
  } else {
    PageGuard result1 = bpm_->VisitPage(parent->ValueAt(location - 1), false);
    PageGuard result2 = bpm_->VisitPage(parent->ValueAt(location + 1), false);
    auto left_sibling = result1.AsMut<TreePage>();
    auto right_sibling = result2.AsMut<TreePage>();
    result_right = (left_sibling->GetSize() < right_sibling->GetSize());
    result = result_right ? std::move(result2) : std::move(result1);
  }
  return result_right;
}

TEMPLATE
auto BPT_TYPE::Insert(const KeyType &key, const ValueType &value) -> bool {
  Trace trace;
  trace.pages_trace[trace.levels] = bpm_->VisitPage(header_page_id_, false);
  trace.place_trace[trace.levels] =
      trace.pages_trace[trace.levels].AsMut<HeaderPage>()->root_page_id_;
  ++trace.levels;
  if (trace.place_trace[0] == INVALID_PAGE_ID) {
    // The tree is empty.
    int new_leaf = bpm_->NewPage();
    auto root_guard = bpm_->VisitPage(new_leaf, false);
    auto cursor_leaf_pointer = root_guard.AsMut<LeafPage>();
    new (cursor_leaf_pointer) LeafPage(leaf_max_size_);
    cursor_leaf_pointer->InsertInPage(0, key, value);
    trace.pages_trace[0].AsMut<HeaderPage>()->root_page_id_ = new_leaf;
    return true;
  }
  /*Situation 2: the tree is not empty.*/
  FindPath(trace, key);
  auto cursor_leaf_pointer = trace.pages_trace[trace.levels].AsMut<LeafPage>();
  int cursor = cursor_leaf_pointer->KeyIndex(key);
  if (cursor < cursor_leaf_pointer->GetSize() &&
      KeyComparator(cursor_leaf_pointer->KeyAt(cursor), key) == 0) {
    return false;
  }
  /*Here comes the adjustment after insert.*/
  int number = cursor_leaf_pointer->InsertInPage(cursor, key, value);
  std::pair<KeyType, page_id_t> info;
  if (number >= leaf_max_size_) {
    info = SplitLeafNode(cursor_leaf_pointer);
    --trace.levels;
    InternalPage *cursor_pointer = nullptr;
    if (trace.levels > 1) {
      cursor_pointer = trace.pages_trace[trace.levels].AsMut<InternalPage>();
      number = cursor_pointer->InsertInPage(trace.place_trace[trace.levels],
                                            info.first, info.second);
      --trace.levels;
      while (number >= internal_max_size_ && trace.level_ > 1) {
        info = SplitInternalNode(cursor_pointer);
        cursor_pointer = trace.pages_trace[trace.levels].AsMut<InternalPage>();
        number = cursor_pointer->InsertInPage(trace.place_trace[trace.levels],
                                              info.first, info.second);
        --trace.level_;
      }
      if (number >= internal_max_size_) {
        info = SplitInternalNode(cursor_pointer);
        cursor = bpm_->NewPage();
        cursor_pointer = bpm_->VisitPage(cursor, false).AsMut<InternalPage>();
        new (cursor_pointer) InternalPage(internal_max_size_);
        cursor_pointer->InsertInPage(0, trace.root_page_id_);
        cursor_pointer->InsertInPage(1, info.first, info.second);
        trace.pages_trace[0].AsMut<HeaderPage>()->root_page_id_ = cursor;
      }
    } else {
      cursor = bpm_->NewPage();
      cursor_pointer = bpm_->VisitPage(cursor, false).AsMut<InternalPage>();
      new (cursor_pointer) InternalPage(internal_max_size_);
      cursor_pointer->InsertInPage(0, trace.root_page_id_);
      cursor_pointer->InsertInPage(1, info.first, info.second);
      trace.pages_trace[0].AsMut<HeaderPage>()->root_page_id_ = cursor;
    }
  }
  return true;
}

TEMPLATE
void BPT_TYPE::Remove(const KeyType &key) {
  Trace trace;
  trace.pages_trace[trace.levels] = bpm_->VisitPage(header_page_id_, false);
  trace.place_trace[trace.levels] =
      trace.pages_trace[trace.levels].AsMut<HeaderPage>()->root_page_id_;
  ++trace.levels;
  if (trace.root_page_id_ == INVALID_PAGE_ID) {
    return;
  }
  FindPath(trace, key);
  /*Situation 2: the tree is not empty.*/
  auto cursor_leaf_pointer = trace.pages_trace[trace.levels].AsMut<LeafPage>();
  int cursor = cursor_leaf_pointer->KeyIndex(key);
  if (cursor >= cursor_leaf_pointer->GetSize() ||
      KeyComparator(cursor_leaf_pointer->KeyAt(cursor), key) != 0) {
    return;
  }
  int number = cursor_leaf_pointer->DeleteInPage(cursor);
  /*Here comes the adjustment after delete.*/
  if (number >= leaf_min_size_ || trace.level_ == 1) {
    if (number == 0) {
      trace.pages_trace[0].AsMut<HeaderPage>()->root_page_id_ = INVALID_PAGE_ID;
    }
    return;
  }
  /*First, we need to merge the leaf, which is lack of items and not the root.*/
  PageGuard sibling;
  PageGuard guard_child = std::move(trace.pages_trace[trace.levels]);
  --trace.levels;
  InternalPage *cursor_parent =
      trace.pages_trace[trace.levels].AsMut<InternalPage>();
  InternalPage *cursor_child = nullptr;
  bool sibling_right = SelectSibling(cursor_parent, key, sibling);
  if (sibling.As<LeafPage>()->GetSize() + number >= leaf_max_size_) {
    KeyType temp = cursor_leaf_pointer->BorrowFromPage(
        sibling.AsMut<LeafPage>(), sibling_right);
    cursor_parent->SetKeyAt(trace.place_trace[trace.levels] +
                                static_cast<int>(sibling_right) - 1,
                            temp);
  } else {
    cursor_leaf_pointer->MergePage(sibling.AsMut<LeafPage>(), sibling_right);
    /*It is the place we start to deal with internal pages.*/
    number = cursor_parent->DeleteInPage(trace.place_trace[trace.levels] +
                                         static_cast<int>(sibling_right) - 1);
    while (number < internal_min_size_ && trace.level_ > 1) {
      guard_child = std::move(trace.pages_trace[trace.levels]);
      cursor_child = guard_child.AsMut<InternalPage>();
      --trace.levels;
      cursor_parent = trace.pages_trace[trace.levels].AsMut<InternalPage>();
      sibling_right = SelectSibling(cursor_parent, key, sibling);
      if (sibling.As<InternalPage>()->GetSize() + number >=
          internal_max_size_) {
        KeyType temp = cursor_child->BorrowFromPage(
            sibling.AsMut<InternalPage>(),
            cursor_parent->KeyAt(trace.place_trace[trace.levels] +
                                 static_cast<int>(sibling_right) - 1),
            sibling_right);
        cursor_parent->SetKeyAt(trace.place_trace[trace.levels] +
                                    static_cast<int>(sibling_right) - 1,
                                temp);
        number = internal_max_size_;
      } else {
        cursor_child->MergePage(
            sibling.AsMut<InternalPage>(),
            cursor_parent->KeyAt(trace.place_trace[trace.levels] +
                                 static_cast<int>(sibling_right) - 1),
            sibling_right);
        number =
            cursor_parent->DeleteInPage(trace.place_trace[trace.levels] +
                                        static_cast<int>(sibling_right) - 1);
      }
    }
    if (trace.level_ == 1 && cursor_parent->GetSize() == 1) {
      trace.pages_trace[0].AsMut<HeaderPage>()->root_page_id_ =
          cursor_parent->ValueAt(0);
    }
  }
}

} // namespace bpt
#endif