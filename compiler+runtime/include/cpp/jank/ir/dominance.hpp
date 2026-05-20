#pragma once

#include <jank/type.hpp>

namespace jank::ir
{
  using identifier = jtl::immutable_string;

  using dominance_map = native_unordered_map<identifier, identifier>;
  using rpo_index_map = native_unordered_map<identifier, usize>;

  struct function;

  void build_dominance(function &fn);

  /* The least common denominator (LCD) can be found by climbing the RPO indices. This
   * is a hack around needing to calculate our own depth values per block. */
  identifier
  lcd(dominance_map const &idom, rpo_index_map const &rpo_index, identifier a, identifier b);

  /* Given a set of block names, find their lowest common dominator using the idom map. */
  identifier lcd(dominance_map const &idom,
                 rpo_index_map const &rpo_index,
                 native_vector<identifier> const &blocks);
}
