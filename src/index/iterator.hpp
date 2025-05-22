#ifndef ITERATOR_HPP
#define ITERATOR_HPP

#include "b_plus_tree_page.hpp"
#include "buffer_pool_manager.hpp"

namespace bpt {

TEMPLATE
class IndexIterator {
  using Leaf = LeafPage<KeyType, ValueType, KeyComparator>;

private:
  BufferPoolManager *bpm_;
  Leaf *page_;
  int place_;

public:
  IndexIterator() = delete;
  IndexIterator(BufferPoolManager *bpm, Leaf *page, int place = 0)
      : bpm_(bpm), page_(page), place_(place){};
  ~IndexIterator() = default;

  auto IsEnd() -> bool { return page_ == nullptr && place_ == 0; };

  auto operator*() -> std::pair<const KeyType &, const ValueType &> {
    return {page_->KeyAt(place_), page_->ValueAt(place_)};
  }

  auto operator++() -> IndexIterator & {
    if (page_ == nullptr) {
      place_ = 0;
    } else {
      ++place_;
      while (place_ >= page_->GetSize()) {
        place_ = 0;
        page_id_t next_page = page_->GetNextPageId();
        if (next_page == INVALID_PAGE_ID) {
          page_ = nullptr;
          break;
        }
        auto guard = bpm_->VisitPage(next_page, false);
        page_ = guard.template AsMut<Leaf>();
      }
    }
    return *this;
  }

  auto operator==(const IndexIterator &itr) const -> bool {
    return (page_ == itr.page_) && (place_ == itr.place_);
  }

  auto operator!=(const IndexIterator &itr) const -> bool {
    return (page_ != itr.page_) || (place_ != itr.place_);
  }

  // A helper function print out the statement of the iterator.
  auto Getinfo() const -> std::pair<Leaf *, int> { return {page_, place_}; }
};
} // namespace bpt

#endif