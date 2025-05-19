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
                    BufferPoolManager *buffer_pool_manager, int leaf_max_size,
                    int internal_max_size)
    : bpm_(buffer_pool_manager), leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size + 1),
      header_page_id_(header_page_id) {
  PageGuard guard = bpm_->VisitPage(header_page_id_);
  auto head_page = guard.AsMut<BPlusTreeHeaderPage>();
  head_page->root_page_id_ = INVALID_PAGE_ID;
  leaf_min_size_ = leaf_max_size_ >> 1;
  internal_min_size_ = internal_max_size_ >> 1;
}

TEMPLATE
auto BPT_TYPE::IsEmpty() const -> bool {
  VisitPageGuard guard = bpm_->VisitPage(header_page_id_);
  auto head_page = guard.As<BPlusTreeHeaderPage>();
  return head_page->root_page_id_ == INVALID_PAGE_ID;
}

TEMPLATE
auto BPT_TYPE::GetValue(const KeyType &key, std::vector<ValueType> *result)
    -> bool {
  // Declaration of context instance.
  ReadPageGuard read_guard = bpm_->ReadPage(header_page_id_);
  page_id_t root_id = read_guard.As<BPlusTreeHeaderPage>()->root_page_id_;
  if (root_id == INVALID_PAGE_ID) {
    return false;
  }
  read_guard = bpm_->ReadPage(root_id);
  auto cursor_pointer = read_guard.As<InternalPage>();
  while (!cursor_pointer->IsLeafPage()) {
    auto temp = bpm_->ReadPage(cursor_pointer->ValueAt(
        cursor_pointer->KeyIndex(key, comparator_) - 1));
    read_guard = std::move(temp);
    cursor_pointer = read_guard.As<InternalPage>();
  }
  auto cursor_leaf_pointer = read_guard.As<LeafPage>();
  int cursor = cursor_leaf_pointer->KeyIndex(key, comparator_);
  if (cursor >= cursor_leaf_pointer->GetSize() ||
      comparator_(cursor_leaf_pointer->KeyAt(cursor), key) != 0) {
    return false;
  }
  result->push_back(cursor_leaf_pointer->ValueAt(cursor));
  return true;
}

TEMPLATE
auto BPT_TYPE::Insert(const KeyType &key, const ValueType &value) -> bool {
  // Declaration of context instance.
  Context ctx;
  ctx.header_page_.emplace(bpm_->VisitPage(header_page_id_));
  auto header_page = ctx.header_page_.value().AsMut<BPlusTreeHeaderPage>();
  ctx.root_page_id_ = header_page->root_page_id_;
  if (ctx.root_page_id_ == INVALID_PAGE_ID) {
    // The tree is empty.
    int new_leaf = bpm_->NewPage();
    auto root_guard = bpm_->VisitPage(new_leaf);
    auto cursor_leaf_pointer = root_guard.AsMut<LeafPage>();
    cursor_leaf_pointer->Init(leaf_max_size_);
    cursor_leaf_pointer->InsertInPage(0, key, value);
    header_page->root_page_id_ = new_leaf;
    ctx.header_page_.reset();
    return true;
  }
  ctx.write_set_.push_back(bpm_->VisitPage(ctx.root_page_id_));
  FindPath(ctx, OperationType::INSERT, key);
  /*Situation 2: the tree is not empty.*/
  auto *cursor_leaf_pointer = ctx.write_set_.back().AsMut<LeafPage>();
  int cursor = cursor_leaf_pointer->KeyIndex(key, comparator_);
  if (cursor < cursor_leaf_pointer->GetSize() &&
      comparator_(cursor_leaf_pointer->KeyAt(cursor), key) == 0) {
    return false;
  }
  /*Here comes the adjustment after insert.*/
  int number = cursor_leaf_pointer->InsertInPage(cursor, key, value);
  std::pair<KeyType, page_id_t> info;
  if (number >= leaf_max_size_) {
    info = SplitLeafNode(cursor_leaf_pointer);
    ctx.write_set_.pop_back();
    InternalPage *cursor_pointer = nullptr;
    if (ctx.level_ > 0) {
      cursor_pointer = ctx.write_set_.back().AsMut<InternalPage>();
      number = cursor_pointer->InsertInPage(ctx.location_set_.back(),
                                            info.first, info.second);
      ctx.location_set_.pop_back();
      --ctx.level_;
      while (number >= internal_max_size_ && ctx.level_ > 0) {
        info = SplitInternalNode(cursor_pointer);
        ctx.write_set_.pop_back();
        cursor_pointer = ctx.write_set_.back().AsMut<InternalPage>();
        number = cursor_pointer->InsertInPage(ctx.location_set_.back(),
                                              info.first, info.second);
        ctx.location_set_.pop_back();
        --ctx.level_;
      }
      if (number >= internal_max_size_) {
        info = SplitInternalNode(cursor_pointer);
        cursor = bpm_->NewPage();
        cursor_pointer = bpm_->VisitPage(cursor).AsMut<InternalPage>();
        cursor_pointer->Init(internal_max_size_);
        cursor_pointer->InsertInPage(0, ctx.root_page_id_);
        cursor_pointer->InsertInPage(1, info.first, info.second);
        auto header_page =
            ctx.header_page_.value().AsMut<BPlusTreeHeaderPage>();
        header_page->root_page_id_ = cursor;
        ctx.header_page_.reset();
      }
    } else {
      cursor = bpm_->NewPage();
      cursor_pointer = bpm_->VisitPage(cursor).AsMut<InternalPage>();
      cursor_pointer->Init(internal_max_size_);
      cursor_pointer->InsertInPage(0, ctx.root_page_id_);
      cursor_pointer->InsertInPage(1, info.first, info.second);
      auto header_page = ctx.header_page_.value().AsMut<BPlusTreeHeaderPage>();
      header_page->root_page_id_ = cursor;
      ctx.header_page_.reset();
    }
  }
  return true;
}

TEMPLATE
void BPT_TYPE::Remove(const KeyType &key) {
  // Declaration of context instance.
  Context ctx;
  ctx.header_page_.emplace(bpm_->VisitPage(header_page_id_));
  auto header_page = ctx.header_page_.value().AsMut<BPlusTreeHeaderPage>();
  ctx.root_page_id_ = header_page->root_page_id_;
  if (ctx.root_page_id_ == INVALID_PAGE_ID) {
    // The tree is empty.
    ctx.header_page_.reset();
    return;
  }
  ctx.write_set_.push_back(bpm_->VisitPage(ctx.root_page_id_));
  FindPath(ctx, OperationType::DELETE, key);
  /*Situation 2: the tree is not empty.*/
  auto *cursor_leaf_pointer = ctx.write_set_.back().AsMut<LeafPage>();
  int cursor = cursor_leaf_pointer->KeyIndex(key, comparator_);
  if (cursor >= cursor_leaf_pointer->GetSize() ||
      comparator_(cursor_leaf_pointer->KeyAt(cursor), key) != 0) {
    return;
  }
  int number = cursor_leaf_pointer->DeleteInPage(cursor);
  /*Here comes the adjustment after delete.*/
  if (number >= leaf_min_size_ || ctx.level_ == 0) {
    if (number == 0) {
      auto header_page = ctx.header_page_.value().AsMut<BPlusTreeHeaderPage>();
      header_page->root_page_id_ = INVALID_PAGE_ID;
      ctx.header_page_.reset();
    }
    return;
  }
  /*First, we need to merge the leaf, as its items not enough and it is not the
   * root.*/
  VisitPageGuard sibling;
  VisitPageGuard guard_child = std::move(ctx.write_set_.back());
  ctx.write_set_.pop_back();
  auto *cursor_parent = ctx.write_set_.back().AsMut<InternalPage>();
  InternalPage *cursor_child = nullptr;
  bool sibling_right = false;
  sibling_right = SelectSibling(cursor_parent, key, sibling);
  if (sibling.As<LeafPage>()->GetSize() + number >= leaf_max_size_) {
    KeyType temp = cursor_leaf_pointer->BorrowFromPage(
        sibling.AsMut<LeafPage>(), sibling_right);
    cursor_parent->SetKeyAt(
        ctx.location_set_.back() + static_cast<int>(sibling_right) - 1, temp);
  } else {
    cursor_leaf_pointer->MergePage(sibling.AsMut<LeafPage>(), sibling_right);
    /*It is the place we start to deal with internal pages.*/
    number = cursor_parent->DeleteInPage(ctx.location_set_.back() +
                                         static_cast<int>(sibling_right) - 1);
    ctx.location_set_.pop_back();
    --ctx.level_;
    while (number < internal_min_size_ && ctx.level_ != 0) {
      guard_child = std::move(ctx.write_set_.back());
      cursor_child = guard_child.AsMut<InternalPage>();
      ctx.write_set_.pop_back();
      cursor_parent = ctx.write_set_.back().AsMut<InternalPage>();
      sibling_right = SelectSibling(cursor_parent, key, sibling);
      if (sibling.As<InternalPage>()->GetSize() + number >=
          internal_max_size_) {
        KeyType temp = cursor_child->BorrowFromPage(
            sibling.AsMut<InternalPage>(),
            cursor_parent->KeyAt(ctx.location_set_.back() +
                                 static_cast<int>(sibling_right) - 1),
            sibling_right);
        cursor_parent->SetKeyAt(ctx.location_set_.back() +
                                    static_cast<int>(sibling_right) - 1,
                                temp);
        number = internal_max_size_;
      } else {
        cursor_child->MergePage(
            sibling.AsMut<InternalPage>(),
            cursor_parent->KeyAt(ctx.location_set_.back() +
                                 static_cast<int>(sibling_right) - 1),
            sibling_right);
        number = cursor_parent->DeleteInPage(
            ctx.location_set_.back() + static_cast<int>(sibling_right) - 1);
        ctx.location_set_.pop_back();
        --ctx.level_;
      }
    }
    if (ctx.level_ == 0 && cursor_parent->GetSize() == 1) {
      auto header_page = ctx.header_page_.value().AsMut<BPlusTreeHeaderPage>();
      header_page->root_page_id_ = cursor_parent->ValueAt(0);
      ctx.header_page_.reset();
    }
  }
}

TEMPLATE
void BPT_TYPE::FindPath(bustub::Context &ctx, OperationType type,
                        const KeyType &key) const {
  page_id_t cursor = 0;
  VisitPageGuard write_guard;
  auto cursor_pointer = ctx.write_set_.back().As<InternalPage>();
  while (!cursor_pointer->IsLeafPage()) {
    cursor = cursor_pointer->KeyIndex(key, comparator_) - 1;
    ctx.location_set_.push_back(cursor + 1);
    ++ctx.level_;
    write_guard = bpm_->VisitPage(cursor_pointer->ValueAt(cursor));
    cursor_pointer = write_guard.As<InternalPage>();
    if (IsNodeSafe(cursor_pointer, type)) {
      ctx.header_page_.reset();
      ctx.write_set_.clear();
    }
    ctx.write_set_.push_back(std::move(write_guard));
  }
}

TEMPLATE
auto BPT_TYPE::SplitLeafNode(LeafPage *cursor)
    -> std::pair<KeyType, page_id_t> {
  page_id_t new_leaf = bpm_->NewPage();
  VisitPageGuard write_guard = bpm_->VisitPage(new_leaf);
  auto cursor_new_leaf = write_guard.AsMut<LeafPage>();
  KeyType middle_key = cursor->SplitPage(cursor_new_leaf);
  cursor_new_leaf->SetNextPageId(cursor->GetNextPageId());
  cursor->SetNextPageId(new_leaf);
  return {middle_key, new_leaf};
}
TEMPLATE
auto BPT_TYPE::SplitInternalNode(InternalPage *cursor)
    -> std::pair<KeyType, page_id_t> {
  page_id_t new_internal = bpm_->NewPage();
  VisitPageGuard write_guard = bpm_->VisitPage(new_internal);
  auto cursor_new_internal = write_guard.AsMut<InternalPage>();
  return {cursor->SplitPage(cursor_new_internal), new_internal};
}

TEMPLATE
auto BPT_TYPE::SelectSibling(InternalPage *parent, const KeyType &key,
                             VisitPageGuard &result) -> bool {
  int location = parent->KeyIndex(key, comparator_) - 1;
  bool result_right = false;
  if (location == 0) {
    result = bpm_->VisitPage(parent->ValueAt(1));
    result_right = true;
  } else if (location == parent->GetSize() - 1) {
    result = bpm_->VisitPage(parent->ValueAt(location - 1));
  } else {
    VisitPageGuard result1 = bpm_->VisitPage(parent->ValueAt(location - 1));
    VisitPageGuard result2 = bpm_->VisitPage(parent->ValueAt(location + 1));
    auto left_sibling = result1.AsMut<BPlusTreePage>();
    auto right_sibling = result2.AsMut<BPlusTreePage>();
    result_right = (left_sibling->GetSize() < right_sibling->GetSize());
    result = result_right ? std::move(result2) : std::move(result1);
  }
  return result_right;
}

} // namespace bpt
#endif