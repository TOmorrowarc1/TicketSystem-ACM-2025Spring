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
  Leaf *page_pointer_;
  int place_;

public:
  IndexIterator() = delete;
  IndexIterator(BufferPoolManager *bpm, PageGuard &&page, int place = 0)
      : bpm_(bpm), page_guard_(std::move(page)), place_(place) {
    page_pointer_ = page.AsMut<LEAF_PAGE_TYPE>();
  };
  ~IndexIterator() = default;

  auto IsEnd() -> bool { return (page_pointer_ == nullptr) && (place_ == 0); };

  auto operator*() -> std::pair<const KeyType &, const ValueType &> {
    return {page_pointer_->KeyAt(place_), page_pointer_->ValueAt(place_)};
  }

  auto operator++() -> IndexIterator & {
    ++place_;
    if (place_ == page_pointer_->GetSize()) {
      place_ = 0;
      if (page_pointer_->GetNextPageId() == -1) {
        page_pointer_ = nullptr;
        return *this;
      }
      page_guard_ = bpm_->ReadPage(page_pointer_->GetNextPageId());
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