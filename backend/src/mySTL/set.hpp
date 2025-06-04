#ifndef SET_HPP
#define SET_HPP

#include <string>

namespace sjtu {

template <typename ValueType, typename Compare> class Set {
public:
  const bool RED = 1;
  const bool BLACK = 0;

private:
  class Node {
  private:
    bool color_;
    Node *parent_;
    Node *left_child_;
    Node *right_child_;
    ValueType *content_;

  public:
    Node()
        : color_(0), parent_(nullptr), left_child_(nullptr),
          right_child_(nullptr), content_(nullptr) {}

    Node(const ValueType &content)
        : color_(0), parent_(nullptr), left_child_(nullptr),
          right_child_(nullptr) {
      content_ = new ValueType(content);
    }

    ~Node() {
      parent_ = left_child_ = right_child_ = nullptr;
      delete content_;
    }

    Node *LeftRotation(Node *parent_before, Node *parent_after) {
      if (parent_after == nullptr) {
        throw std::exception();
      }
      parent_after->parent_ = parent_before->parent_;
      if (parent_before->parent_ != nullptr) {
        if (parent_before->parent_->left_child_ == parent_before) {
          parent_before->parent_->left_child_ = parent_after;
        } else {
          parent_before->parent_->right_child_ = parent_after;
        }
      }
      parent_before->right_child_ = parent_after->left_child_;
      if (parent_after->left_child_ != nullptr) {
        parent_after->left_child_->parent_ = parent_before;
      }
      parent_after->left_child_ = parent_before;
      parent_before->parent_ = parent_after;
      return parent_after;
    }

    Node *RightRotation(Node *parent_before, Node *parent_after) {
      if (parent_after == nullptr) {
        throw std::exception();
      }
      parent_after->parent_ = parent_before->parent_;
      if (parent_before->parent_ != nullptr) {
        if (parent_before->parent_->left_child_ == parent_before) {
          parent_before->parent_->left_child_ = parent_after;
        } else {
          parent_before->parent_->right_child_ = parent_after;
        }
      }
      parent_before->left_child_ = parent_after->right_child_;
      if (parent_after->right_child_ != nullptr) {
        parent_after->right_child_->parent_ = parent_before;
      }
      parent_after->right_child_ = parent_before;
      parent_before->parent_ = parent_after;
      return parent_after;
    }

    void ExchangeWithEmpty(Node *target, Node *empty) {
      empty->parent_ = target->parent_;
      empty->left_child_ = target->left_child_;
      empty->right_child_ = target->right_child_;
      if (empty->parent_ != nullptr) {
        if (target->parent_->left_child_ == target) {
          target->parent_->left_child_ = empty;
        } else {
          target->parent_->right_child_ = empty;
        }
      }
      if (empty->left_child_ != nullptr) {
        empty->left_child_->parent_ = empty;
      }
      if (empty->right_child_ != nullptr) {
        empty->right_child_->parent_ = empty;
      }
    }

    void Swap(Node *high, Node *low, Node *sentinar) {
      bool temp_color = high->color_;
      high->color_ = low->color_;
      low->color_ = temp_color;
      ExchangeWithEmpty(high, sentinar);
      ExchangeWithEmpty(low, high);
      ExchangeWithEmpty(sentinar, low);
    }

    friend class Set;
  };

  Node *root_;
  Node *sentinar_;
  int nodes_num_;

public:
  Set() : root_(nullptr), nodes_num_(0) { sentinar_ = new Node(); }

  /*Copy the nodes recursively*/
  auto Copy(Node *root, Node *other) -> Node * {
    root = new Node(*(other->content_));
    root->color_ = other->color_;
    if (other->left_child_ != nullptr) {
      root->left_child_ = Copy(root->left_child_, other->left_child_);
      root->left_child_->parent_ = root;
    }
    if (other->right_child_ != nullptr) {
      root->right_child_ = Copy(root->right_child_, other->right_child_);
      root->right_child_->parent_ = root;
    }
    return root;
  }

  Set(const Set &other) {
    nodes_num_ = other.nodes_num_;
    root_ = nullptr;
    if (other.nodes_num_ != 0) {
      root_ = Copy(root_, other.root_);
    }
  }

  void Erase(Node *root) {
    if (root == nullptr) {
      return;
    }
    Erase(root->left_child_);
    Erase(root->right_child_);
    --nodes_num_;
    delete root;
  }

  void Clear() {
    if (nodes_num_ != 0) {
      Erase(root_);
    }
    root_ = nullptr;
    delete sentinar_;
    sentinar_ = nullptr;
    nodes_num_ = 0;
  }

  ~Set() {
    Erase(root_);
    delete sentinar_;
  }

  auto operator=(const Set &other) -> Set & {
    if (root_ == other.root_) {
      return *this;
    }
    Erase(root_);
    root_ = nullptr;
    root_ = Copy(root_, other.root_);
    return *this;
  }

  auto Predecessor(const Node *base) const -> Node * {
    Node *target = (Node *)(base);
    if (target->left_child_ != nullptr) {
      target = target->left_child_;
      while (target->right_child_ != nullptr) {
        target = target->right_child_;
      }
      return target;
    }
    while (target != root_ && target->parent_->left_child_ == target) {
      target = target->parent_;
    }
    return target->parent_;
  }

  auto Successor(const Node *base) const -> Node * {
    Node *target = (Node *)(base);
    if (target->right_child_ != nullptr) {
      target = target->right_child_;
      while (target->left_child_ != nullptr) {
        target = target->left_child_;
      }
      return target;
    }
    while (target != root_ && target->parent_->right_child_ == target) {
      target = target->parent_;
    }
    return target->parent_;
  }

  auto Search(const ValueType &value) const -> Node * {
    Node *target = root_;
    Node *parent = nullptr;
    while (target != nullptr) {
      if (Compare()(value, *target->content_) == 0) {
        return target;
      }
      if (Compare()(value, *target->content_) < 0) {
        parent = target;
        target = target->left_child_;
      } else {
        parent = target;
        target = target->right_child_;
      }
    }
    return parent;
  }

  auto Exist(const ValueType value) const -> bool {
    if (root_ == nullptr) {
      return false;
    }
    Node *place = Search(value);
    return Compare()(value, *place->content_) == 0;
  }

  void InsertMaintain(Node *target) {
    Node *parent = nullptr;
    Node *grandparent = nullptr;
    Node *uncle = nullptr;
    while (target != root_ && target->parent_->color_ != BLACK) {
      /*If the parent is RED, the grandparent(if existed) must BLACK and
      there will be two cases for analysis:
        1. The uncle is BLACK, which is equivlant to the target is Inserted in a
      2-item B-Tree node, resulting in rotations and repainting to make a 3-item
      B-Tree Node.
        2. The uncle is RED, which means that the target Inserting in a 3-item
      full B-Tree node, resulting in repainting equals to a split.*/
      parent = target->parent_;
      grandparent = parent->parent_;
      if (parent == grandparent->left_child_) {
        uncle = grandparent->right_child_;
      } else {
        uncle = grandparent->left_child_;
      }
      if (uncle != nullptr && uncle->color_ == RED) {
        parent->color_ = BLACK;
        uncle->color_ = BLACK;
        grandparent->color_ = RED;
        target = grandparent;
      } else {
        if (grandparent->left_child_ == parent) {
          if (parent->right_child_ == target) {
            Node *temp = parent;
            parent = target->LeftRotation(parent, target);
            target = temp;
          }
          parent->color_ = BLACK;
          grandparent->color_ = RED;
          parent->RightRotation(grandparent, parent);
        } else {
          if (parent->left_child_ == target) {
            Node *temp = parent;
            parent = target->RightRotation(parent, target);
            target = temp;
          }
          parent->color_ = BLACK;
          grandparent->color_ = RED;
          parent->LeftRotation(grandparent, parent);
        }
      }
    }
    while (root_->parent_ != nullptr) {
      root_ = root_->parent_;
    }
    root_->color_ = BLACK;
  }

  auto Insert(const ValueType &value) -> bool {
    if (root_ == nullptr) {
      root_ = new Node(value);
      nodes_num_ = 1;
      return true;
    }
    Node *place = Search(value);
    if (Compare()(value, *place->content_) == 0) {
      return false;
    }
    Node *target = new Node(value);
    target->color_ = RED;
    target->parent_ = place;
    if (Compare()(value, *place->content_) < 0) {
      place->left_child_ = target;
    } else {
      place->right_child_ = target;
    }
    InsertMaintain(target);
    ++nodes_num_;
    return true;
  }

  void EraseMaintain(Node *target) {
    /*
      If remove a node on the leaf, adjustment of the tree falls in several
    cases:
      1. The target is RED, which means that we Erase an item from a 2/3 item
    B-Tree node. The only task is to throw it away and change the pointer.
      2. The target is BLACK, which means that we kill a B-Tree node. We need to
    analysis its brother B-Tree node so first we rotate to make its sibling a
    BLACK node that equivalent to a sibling in B-Tree instead of its parent.
      2.1 The brother has at least one RED child: make sure the child on the
    opposite direction towards the target is RED, then rotate to make the
    sibling uplift and repaint.
      2.2 The sibling has two BLACK child: there is no abundant child of the
    sibling, so the only choice is to merge the node. Rotate the sibling
    upwards, repaint and adjust the tree recursively.
    */
    Node *parent = nullptr;
    Node *sibling = nullptr;
    while (target != root_ && (target == nullptr || target->color_ == BLACK)) {
      parent = target->parent_;
      if (parent->left_child_ == target) {
        sibling = parent->right_child_;
        if (sibling->color_ == RED) {
          parent->color_ = RED;
          sibling->color_ = BLACK;
          sibling->LeftRotation(parent, sibling);
          sibling = parent->right_child_;
        }
        if (sibling->right_child_ != nullptr &&
            sibling->right_child_->color_ == RED) {
          sibling->color_ = parent->color_;
          parent->color_ = BLACK;
          sibling->right_child_->color_ = BLACK;
          sibling->LeftRotation(parent, sibling);
          break;
        }
        if (sibling->left_child_ != nullptr &&
            sibling->left_child_->color_ == RED) {
          sibling = sibling->RightRotation(sibling, sibling->left_child_);
          sibling->color_ = parent->color_;
          parent->color_ = BLACK;
          sibling->right_child_->color_ = BLACK;
          sibling->LeftRotation(parent, sibling);
          break;
        }
        if (parent->color_ == RED) {
          parent->color_ = BLACK;
          sibling->color_ = RED;
          break;
        }
        sibling->color_ = RED;
        target = parent;
      } else {
        sibling = parent->left_child_;
        if (sibling->color_ == RED) {
          parent->color_ = RED;
          sibling->color_ = BLACK;
          sibling->RightRotation(parent, sibling);
          sibling = parent->left_child_;
        }
        if (sibling->left_child_ != nullptr &&
            sibling->left_child_->color_ == RED) {
          sibling->color_ = parent->color_;
          parent->color_ = BLACK;
          sibling->left_child_->color_ = BLACK;
          sibling->RightRotation(parent, sibling);
          break;
        }
        if (sibling->right_child_ != nullptr &&
            sibling->right_child_->color_ == RED) {
          sibling = sibling->LeftRotation(sibling, sibling->right_child_);
          sibling->color_ = parent->color_;
          parent->color_ = BLACK;
          sibling->left_child_->color_ = BLACK;
          sibling->RightRotation(parent, sibling);
          break;
        }
        if (parent->color_ == RED) {
          parent->color_ = BLACK;
          sibling->color_ = RED;
          break;
        }
        sibling->color_ = RED;
        target = parent;
      }
    }
    while (root_->parent_ != nullptr) {
      root_ = root_->parent_;
    }
    root_->color_ = BLACK;
  }

  auto Erase(const ValueType &value) -> bool {
    if (nodes_num_ <= 1) {
      delete root_;
      root_ = nullptr;
      nodes_num_ = 0;
      return true;
    }
    /*
      If the node is on the leaf, then we can Erase it then maintain the R-B
    characteristic. Otherwise, we need to find its predecessor or successor
    and Swap their location, then Erase it.
    */
    Node *place = Search(value);
    Node *record = place;
    if (Compare()(value, *place->content_) != 0) {
      return false;
    }
    if (place->left_child_ != nullptr) {
      place = Predecessor(place);
      place->Swap(record, place, sentinar_);
      while (root_->parent_ != nullptr) {
        root_ = root_->parent_;
      }
      place = record;
      if (place->left_child_ != nullptr) {
        place->Swap(place, place->left_child_, sentinar_);
      }
    } else if (place->right_child_ != nullptr) {
      place = Successor(place);
      place->Swap(record, place, sentinar_);
      while (root_->parent_ != nullptr) {
        root_ = root_->parent_;
      }
      place = record;
      if (place->right_child_ != nullptr) {
        place->Swap(place, place->right_child_, sentinar_);
      }
    }
    EraseMaintain(place);
    if (place->parent_->left_child_ == place) {
      place->parent_->left_child_ = nullptr;
    } else {
      place->parent_->right_child_ = nullptr;
    }
    --nodes_num_;
    delete place;
    return true;
  }
};

} // namespace sjtu

#endif