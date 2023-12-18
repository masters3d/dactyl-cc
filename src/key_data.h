#pragma once

#include "util/key.h"
#include "util/transform.h"

namespace scad {

// Key positioning data and description of layout and grouping of keys.
struct KeyData {
  KeyData(TransformList key_origin);

  TransformList origin_for_bowl;

  // This is the key that every other key is relative to.
  Key* origin_key;

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

  Key key_4_0;
  Key key_4_1;
  Key key_4_2;
  Key key_4_3;
  Key key_4_4;

  Key key_thumb_0_0;
  Key key_thumb_0_1;
  Key key_thumb_0_2;
  Key key_thumb_0_3;
  Key key_thumb_0_4;
  Key key_thumb_0_5;

  KeyGrid grid {{
    { &key_0_0,  &key_0_1,  &key_0_2,  &key_0_3,  &key_0_4,  &key_0_5},
    { &key_1_0,  &key_1_1,  &key_1_2,  &key_1_3,  &key_1_4,  &key_1_5},
    { &key_2_0,  &key_2_1,  &key_2_2,  &key_2_3,  &key_2_4,  &key_2_5},
    { &key_3_0,  &key_3_1,  &key_3_2,  &key_3_3,  &key_3_4,  &key_3_5},
      {&key_4_0, & key_4_1, &key_4_2, &key_4_3, &key_4_4, nullptr},
  }};

  std::vector<Key*> thumb_keys() {
    return {&key_thumb_0_1, &key_thumb_0_0, &key_thumb_0_5, &key_thumb_0_4, &key_thumb_0_3, &key_thumb_0_2};
  }

  std::vector<Key*> bowl_keys() {
    return grid.keys();
  }

  std::vector<Key*> all_keys() {
    std::vector<Key*> keys;
    for (Key* key : thumb_keys()) {
      key->description();
      keys.push_back(key);
    }
    for (Key* key : grid.keys()) {
      key->description();
      keys.push_back(key);
    }
    return keys;
  }
};

}  // namespace scad
