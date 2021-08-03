//
// Created by TD on 2021/7/30.
//

#pragma once
#include <DoodleLib/DoodleLib_fwd.h>

#include <boost/intrusive/intrusive_fwd.hpp>
#include <boost/intrusive/link_mode.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/pack_options.hpp>
#include <boost/intrusive/set.hpp>
#include <boost/intrusive/trivial_value_traits.hpp>
#include <boost/signals2.hpp>

namespace doodle {

using set_base_hook = boost::intrusive::set_base_hook<
    boost::intrusive::link_mode<boost::intrusive::safe_link>>;

class tree_node;
using tree_node_ptr = std::shared_ptr<tree_node>;

template <class... type_arg>
tree_node_ptr make_tree(type_arg&&... in_arg);

namespace details {
class DOODLELIB_API tree_node_destroy : public std::default_delete<tree_node> {
 public:
  tree_node_destroy() = default;
  void operator()(tree_node* in_ptr);
};

template <class value, class container>
class DOODLELIB_API observe : public no_copy {
 public:
  using _value_type    = value;
  using container_type = container;
  observe()            = default;

  /**
   * @brief 开始清理时发出的信号
   */
  boost::signals2::signal<void()> sig_begin_clear;
  /**
   * @brief 开始插入时发出的信号
   */
  boost::signals2::signal<void(const _value_type& val)> sig_begin_insert;
  /**
   * @brief 开始擦除时发出的信号
   */
  boost::signals2::signal<void(const _value_type& val)> sig_begin_erase;
  /**
   * @brief 开始追加时发出的信号
   */
  boost::signals2::signal<void(const _value_type& val)> sig_begin_push_back;
  /**
   * @brief 开始调整大小时发出的信号
   */
  boost::signals2::signal<void(std::size_t where)> sig_begin_resize;
  /**
   * @brief 开始交换容器内容时发出的信号
   */
  boost::signals2::signal<void(const container_type& val)> sig_begin_swap;

  /**
   * @brief 排序开始回调
   *
   */
  boost::signals2::signal<void(const container_type& val)> sig_begin_sort;

  //----------------------------------------------------------------------------

  /**
   * @brief 清理时发出的信号
   */
  boost::signals2::signal<void()> sig_clear;
  /**
   * @brief 插入时发出的信号
   */
  boost::signals2::signal<void(const _value_type& val)> sig_insert;
  /**
   * @brief 擦除时发出的信号
   */
  boost::signals2::signal<void(const _value_type& val)> sig_erase;
  /**
   * @brief 追加时发出的信号
   */
  boost::signals2::signal<void(const _value_type& val)> sig_push_back;
  /**
   * @brief 调整大小时发出的信号
   */
  boost::signals2::signal<void(std::size_t where)> sig_resize;
  /**
   * @brief 交换容器内容时发出的信号
   */
  boost::signals2::signal<void(const container_type& val)> sig_swap;

  /**
   * @brief 排序回调
   *
   */
  boost::signals2::signal<void(const container_type& val)> sig_sort;
};
}  // namespace details

/**
 * @brief 这是一个树结构
 */
class DOODLELIB_API tree_node : public std::enable_shared_from_this<tree_node>,
                                public set_base_hook,
                                public details::no_copy {
  friend details::tree_node_destroy;
  tree_node();
  explicit tree_node(tree_node* in_parent, MetadataPtr in_data);
  explicit tree_node(const tree_node_ptr& in_parent, MetadataPtr in_data);

  void insert_private(const tree_node_ptr& in_);

 public:
  ~tree_node();
  using tree_node_ptr = std::shared_ptr<tree_node>;

  using child_set = boost::intrusive::set<
      tree_node,
      boost::intrusive::base_hook<tree_node>,
      boost::intrusive::constant_time_size<false>>;
  using child_set_owner = std::set<tree_node_ptr>;

  template <class... type_arg>
  static tree_node_ptr make_this(type_arg&&... in_arg) {
    auto tmp = std::shared_ptr<tree_node>{
        new tree_node{std::forward<type_arg>(in_arg)...},
        details::tree_node_destroy()};
    /// 非默认构造, 这个时候可以直接检查父物体是否为空后插入
    if constexpr (sizeof...(in_arg) > 1) {
      if (tmp->has_parent())
        tmp->parent->insert_private(tmp);
    }
    return tmp;
  };

  [[nodiscard]] bool is_root() const;
  [[nodiscard]] bool has_parent() const;
  [[nodiscard]] tree_node_ptr get_parent() const;
  [[nodiscard]] const child_set& get_children() const;
  [[nodiscard]] bool has_child() const;
  void insert(const tree_node_ptr& in_);
  //  void insert(tree_node& in_);
  void remove(const tree_node_ptr& in_);
  //  void remove(tree_node& in_);
  void clear();

  bool operator==(const tree_node& in_rhs) const;
  bool operator!=(const tree_node& in_rhs) const;

  bool operator<(const tree_node& in_rhs) const;
  bool operator>(const tree_node& in_rhs) const;
  bool operator<=(const tree_node& in_rhs) const;
  bool operator>=(const tree_node& in_rhs) const;

  operator MetadataPtr&();
  MetadataPtr& get();

  operator const MetadataPtr&() const;
  const MetadataPtr& get() const;

 private:
  tree_node* parent;
  MetadataPtr data;
  child_set child_item;
  child_set_owner child_owner;
  details::observe<MetadataPtr, child_set> sig_class;
};

template <class... type_arg>
tree_node_ptr make_tree(type_arg&&... in_arg) {
  return tree_node::make_this(std::forward<type_arg>(in_arg)...);
}

// class tree_meta;
// class tree_meta {
//  public:
//   using tree_meta_ptr      = std::shared_ptr<tree_meta>;
//   using tree_meta_weak_ptr = std::weak_ptr<tree_meta>;
//
//  private:
//   tree_meta_weak_ptr parent;
//   std::set<tree_meta_ptr> child_item;
//   MetadataPtr data;
//
//  public:
//   tree_meta() = default;
//   explicit tree_meta(tree_meta_ptr& in_meta, MetadataPtr in_metadata);
//   //  bool has_parent();
//   //  bool is_root();
// };
}  // namespace doodle
