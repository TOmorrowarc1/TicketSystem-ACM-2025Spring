#ifndef B_PLUS_TREE_PAGE_HPP
#define B_PLUS_TREE_PAGE_HPP
namespace bpt {

#define TEMPLATE                                                               \
  template <typename KeyType, typename ValueType, typename KeyComparator>
#define LEAF_PAGE_TYPE LeafPage<KeyType, ValueType, KeyComparator>
#define INTERNAL_PAGE_TYPE InternalPage<KeyType, ValueType, KeyComparator>
class TreePage {
public:
  enum class PageType : bool { LEAF = false, INTERNAL };

private:
  int size_;
  int max_size_;
  PageType page_type_;

public:
  TreePage() : size_(0), max_size_(0), page_type_(PageType::LEAF) {}
  TreePage(int maxsize, PageType page_type)
      : size_(0), max_size_(maxsize), page_type_(page_type) {}
  TreePage(const TreePage &other) = delete;
  ~TreePage() = default;

  auto IsLeafPage() const -> bool { return page_type_ == PageType::LEAF; }
  auto GetPageType() const -> PageType { return page_type_; }
  void SetPageType(PageType page_type) { page_type_ = page_type; }

  auto GetSize() const -> int { return size_; }
  void SetSize(int size) { size_ = size; }

  auto GetMaxSize() const -> int { return max_size_; }
  void SetMaxSize(int size) { max_size_ = size; }
  auto GetMinSize() const -> int { return max_size_ >> 1; }
};

TEMPLATE
class LeafPage : public TreePage {
private:
  KeyType key_array_[(4096 - 16) / (sizeof(KeyType) + sizeof(ValueType))];
  ValueType value_array_[(4096 - 16) / (sizeof(KeyType) + sizeof(ValueType))];
  page_id_t next_page_id_;

public:
  LeafPage() = delete;
  LeafPage(int maxsize = (4096 - 16) / (sizeof(KeyType) + sizeof(ValueType)))
      : TreePage(maxsize, PageType::LEAF) {}
  LeafPage(const BPlusTreeLeafPage &other) = delete;

  auto GetNextPageId() const -> page_id_t { return next_page_id_; };
  void SetNextPageId(page_id_t next_page_id) { next_page_id_ = next_page_id; }

  auto KeyAt(int index) const -> KeyType { return key_array_[index]; }
  auto ValueAt(int index) const -> ValueType { return value_array_[index]; }
  auto KeyIndex(const KeyType &target) const -> int;

  auto InsertInPage(int index, const KeyType &target_key,
                    const ValueType &target_value) -> int;
  auto SplitPage(LeafPage *empty) -> KeyType;

  auto DeleteInPage(int index) -> int;
  auto BorrowFromPage(LeafPage *sibling, bool sibling_right) -> KeyType;
  void MergePage(LeafPage *sibling, bool sibling_right);
};

TEMPLATE
auto LEAF_PAGE_TYPE::KeyIndex(const KeyType &target) const -> int {
  int left = 0;
  int right = GetSize();
  int middle = 0;
  while (left < right) {
    middle = (left + right) >> 1;
    if (KeyComparator(key_array_[middle], target) < 0) {
      left = middle + 1;
    } else {
      right = middle;
    }
  }
  return left;
}

TEMPLATE
auto LEAF_PAGE_TYPE::InsertInPage(int index, const KeyType &target_key,
                                  const ValueType &target_value) -> int {
  std::memmove(key_array_ + index + 1, key_array_ + index,
               (GetSize() - index) * sizeof(KeyType));
  key_array_[index] = target_key;
  std::memmove(value_array_ + index + 1, value_array_ + index,
               (GetSize() - index) * sizeof(ValueType));
  value_array_[index] = target_value;
  SetSize(GetSize() + 1);
  return GetSize();
}

TEMPLATE
auto LEAF_PAGE_TYPE::SplitPage(LeafPage *empty) -> KeyType {
  empty->Init(GetMaxSize());
  empty->SetPageType(GetPageType());
  empty->SetSize(GetSize() >> 1);
  SetSize(GetSize() - empty->GetSize());
  std::memcpy(empty->key_array_, key_array_ + GetSize(),
              empty->GetSize() * sizeof(KeyType));
  std::memcpy(empty->value_array_, value_array_ + GetSize(),
              empty->GetSize() * sizeof(ValueType));
  return empty->key_array_[0];
}

TEMPLATE
auto LEAF_PAGE_TYPE::DeleteInPage(int index) -> int {
  std::memmove(key_array_ + index, key_array_ + index + 1,
               (GetSize() - index - 1) * sizeof(KeyType));
  std::memmove(value_array_ + index, value_array_ + index + 1,
               (GetSize() - index - 1) * sizeof(ValueType));
  SetSize(GetSize() - 1);
  return GetSize();
}

TEMPLATE
auto LEAF_PAGE_TYPE::BorrowFromPage(LeafPage *sibling, bool sibling_right)
    -> KeyType {
  sibling->SetSize(sibling->GetSize() - 1);
  SetSize(GetSize() + 1);
  if (sibling_right) {
    key_array_[GetSize() - 1] = sibling->key_array_[0];
    value_array_[GetSize() - 1] = sibling->value_array_[0];
    std::memmove(sibling->key_array_, sibling->key_array_ + 1,
                 sibling->GetSize() * sizeof(KeyType));
    std::memmove(sibling->value_array_, sibling->value_array_ + 1,
                 sibling->GetSize() * sizeof(ValueType));
    return sibling->key_array_[0];
  }
  std::memmove(key_array_ + 1, key_array_, (GetSize() - 1) * sizeof(KeyType));
  std::memmove(value_array_ + 1, value_array_,
               (GetSize() - 1) * sizeof(ValueType));
  key_array_[0] = sibling->key_array_[sibling->GetSize()];
  value_array_[0] = sibling->value_array_[sibling->GetSize()];
  return key_array_[0];
}

TEMPLATE
void LEAF_PAGE_TYPE::MergePage(LeafPage *sibling, bool sibling_right) {
  if (sibling_right) {
    std::memmove(key_array_ + GetSize(), sibling->key_array_,
                 sibling->GetSize() * sizeof(KeyType));
    std::memmove(value_array_ + GetSize(), sibling->value_array_,
                 sibling->GetSize() * sizeof(ValueType));
    SetSize(GetSize() + sibling->GetSize());
    SetNextPageId(sibling->GetNextPageId());
    sibling->SetSize(0);
  } else {
    std::memmove(sibling->key_array_ + sibling->GetSize(), key_array_,
                 GetSize() * sizeof(KeyType));
    std::memmove(sibling->value_array_ + sibling->GetSize(), value_array_,
                 GetSize() * sizeof(ValueType));
    sibling->SetSize(sibling->GetSize() + GetSize());
    sibling->SetNextPageId(GetNextPageId());
    SetSize(0);
  }
};

TEMPLATE
class InternalPage : public TreePage {
private:
  KeyType key_array_[(4096 - 16) / (sizeof(KeyType) + sizeof(ValueType))];
  ValueType value_array_[(4096 - 16) / (sizeof(KeyType) + sizeof(ValueType))];

public:
  auto KeyAt(int index) const -> KeyType;
  auto ValueAt(int index) const -> ValueType;
  auto KeyIndex(const KeyType &key) const -> int;

  auto InsertInPage(int index, const KeyType &target_key,
                    const ValueType &page_id) -> int;
  auto InsertInPage(int index, const ValueType &page_id) -> int;
  auto SplitPage(InternalPage *empty) -> KeyType;

  auto DeleteInPage(int index) -> int;
  auto BorrowFromPage(InternalPage *sibling, const KeyType &middle,
                      bool sibling_right) -> KeyType;
  void MergePage(InternalPage *sibling, const KeyType &middle,
                 bool sibling_right);
};

} // namespace bpt
#endif