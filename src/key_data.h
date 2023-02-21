#pragma once

#include "key.h"
#include "transform.h"

namespace scad {

// Key positioning data and description of layout and grouping of keys.
struct KeyData {
  KeyData(TransformList origin);

  Key key_0_0;
  Key key_0_1;
  Key key_0_2;
  Key key_0_3;
  Key key_0_4;
  Key key_0_5;

  Key key_1_0;
  Key key_1_1;
  Key key_1_2;
  Key key_1_3;
  Key key_1_4;
  Key key_1_5;

  Key key_2_0;
  Key key_2_1;
  Key key_2_2;
  Key key_2_3;
  Key key_2_4;
  Key key_2_5;

  Key key_3_0;
  Key key_3_1;
  Key key_3_2;
  Key key_3_3;
  Key key_3_4;
  Key key_3_5;

  Key key_4_1;
  Key key_4_2;
  Key key_4_3;
  Key key_4_4;

  Key key_backspace;
  Key key_delete;
  Key key_end;
  Key key_home;
  Key key_ctrl;
  Key key_alt;

  // clang-format off
  KeyGrid grid {{
    { &key_0_0,  &key_0_1,  &key_0_2,  &key_0_3,  &key_0_4,  &key_0_5},
    { &key_1_0,  &key_1_1,  &key_1_2,  &key_1_3,  &key_1_4,  &key_1_5},
    { &key_2_0,  &key_2_1,  &key_2_2,  &key_2_3,  &key_2_4,  &key_2_5},
    { &key_3_0,  &key_3_1,  &key_3_2,  &key_3_3,  &key_3_4,  &key_3_5},
    { nullptr,   &key_4_1,  &key_4_2,  &key_4_3,  &key_4_4,  nullptr},
  }};
  // clang-format on

  std::vector<Key*> thumb_keys() {
    return {&key_delete, &key_backspace, &key_ctrl, &key_alt, &key_home, &key_end};
  }

  std::vector<Key*> all_keys() {
    std::vector<Key*> keys;
    for (Key* key : thumb_keys()) {
      keys.push_back(key);
    }
    for (Key* key : grid.keys()) {
      keys.push_back(key);
    }
    return keys;
  }
};

}  // namespace scad
