#ifndef ITERATOR_HPP
#define ITERATOR_HPP

#include "b_plus_tree_page.hpp"
#include "buffer_pool_manager.hpp"

namespace bpt {

TEMPLATE
class IndexIterator {

private:
  PageGuard page_guard_;
  BufferPoolManager *bpm_;
  const LEAF_PAGE_TYPE *page_pointer_;
  int place_;

public:
  IndexIterator() : bpm_(nullptr), page_pointer_(nullptr), place_(0){};
  IndexIterator(BufferPoolManager *bpm, PageGuard &&page, int place = 0)
      : bpm_(bpm), page_guard_(std::move(page)), place_(place) {
    page_pointer_ = page_guard_.As<LEAF_PAGE_TYPE>();
  };
  IndexIterator(IndexIterator &&other) {
    bpm_ = other.bpm_;
    other.bpm_ = nullptr;
    page_guard_ = std::move(other.page_guard_);
    place_ = other.place_;
    page_pointer_ = other.page_pointer_;
    other.page_pointer_ = nullptr;
  }
  ~IndexIterator() = default;

  auto IsEnd() -> bool { return (page_pointer_ == nullptr) && (place_ == 0); };

  auto operator*() -> std::pair<KeyType, ValueType> {
    return {page_pointer_->KeyAt(place_), page_pointer_->ValueAt(place_)};
  }

  auto operator++() -> IndexIterator & {
    ++place_;
    if (place_ >= page_pointer_->GetSize()) {
      place_ = 0;
      if (page_pointer_->GetNextPageId() == -1) {
        page_pointer_ = nullptr;
        return *this;
      }
      page_guard_ = bpm_->VisitPage(page_pointer_->GetNextPageId(), true);
      page_pointer_ = page_guard_.As<LEAF_PAGE_TYPE>();
    }
    return *this;
  }

  auto operator==(const IndexIterator &itr) const -> bool {
    return (page_pointer_ == itr.page_pointer_) && (place_ == itr.place_);
  }

  auto operator!=(const IndexIterator &itr) const -> bool {
    return (page_pointer_ != itr.page_pointer_) || (place_ != itr.place_);
  }
};
} // namespace bpt

#endif