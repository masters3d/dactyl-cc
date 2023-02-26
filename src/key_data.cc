#include "key_data.h"

#include <glm/glm.hpp>
#include "key.h"
#include "scad.h"
#include "transform.h"

namespace scad {
namespace {

// The direct distance between switch tops in the bowl.
constexpr double kBowlKeySpacing = 18;

// finger pinky
constexpr double kColumn0Radius = 70;
constexpr double kColumn1Radius = 70;
// finger ring
constexpr double kColumn2Radius = 65;
// finger middle
constexpr double kColumn3Radius = 55;
// finger index
constexpr double kColumn4Radius = 70;
constexpr double kColumn5Radius = 65;

enum class RotationDirection { UP, DOWN};

RotationDirection rotationDirectionUp = RotationDirection::UP;
RotationDirection rotationDirectionDown = RotationDirection::DOWN;

// Rotates a key about the x axis until it has traveled the direct distance (not on the arc).
Key GetXAxisRotatedKey(double radius, RotationDirection rotationDirection) {
  double distance = kBowlKeySpacing;
  double rotation_direction = rotationDirection == RotationDirection::UP ? 1.0 : -1.0;
  double degrees = 1;

  // Use precomputed numbers for known radiuses to speed up execution.
  if (radius == 50) {
    degrees = 20.740;
  }
  if (radius == 55) {
    degrees = 18.840;
  }
  if (radius == 60) {
    degrees = 17.26;
  }
  if (radius == 65) {
    degrees = 15.920;
  }
  if (radius == 70) {
    degrees = 14.780;
  }

  bool is_first = true;
  while (true) {
    Key k;
    k.local_transforms.TranslateZ(-1 * radius)
        .RotateX(rotation_direction * degrees)
        .TranslateZ(radius);
    glm::vec3 point = k.GetTransforms().Apply(kOrigin);
    float current_distance = glm::length(point);
    if (current_distance > distance) {
      if (!is_first) {
        printf("Cumputed degrees %.3f for radius %.3f\n", degrees, radius);
      }
      return k;
    }
    degrees += .01;
    is_first = false;
  }
}

}  // namespace

KeyData::KeyData(TransformList key_origin) {
  double anchor_x = 26.40;
  double anchor_y = 50.32;
  double anchor_z = 17.87;
  // This cotrolls the tilting of the whole bowl
  double anchor_rotate_y = -15;

  // Thumb cluster is indepenently position from bowl keys.
  double anchor_thumb_x = 60;
  double anchor_thumb_y = -9.18;
  double anchor_thumb_z = 42.83;
  double anchor_thumb_rotate_x = -21;
  double anchor_thumb_rotate_y = 12;
  double anchor_thumb_rotate_z = -4.5;

  //
  // Thumb keys
  //

  key_thumb_5_0.Configure([&](Key& k) {
    k.name = "key_backspace";
    k.SetParent(key_origin);
    k.SetPosition(anchor_thumb_x, anchor_thumb_y, anchor_thumb_z);
    k.t().rz = anchor_thumb_rotate_x;
    k.t().rx = anchor_thumb_rotate_y;
    k.t().ry = anchor_thumb_rotate_z;
  });

  //
  // Main bowl keys
  //

  // All keys in the dish are relative to the homerow which is row index 2.
  // The center of the home row is set to column 3. (key_2_3)  In qwerty this is the letter D.
  key_2_3.Configure([&](Key& k) {
    k.name = "d";
    k.SetParent(key_origin);
    k.SetPosition(anchor_x, anchor_y, anchor_z);
    k.t().ry = anchor_rotate_y;
  });

  key_2_4.Configure([&](Key& k) {
    k.name = "f";

    // Absolute:
    // k.SetPosition(44.3, 49.37, 28.1);
    // k.t().ry = -20;

    k.SetParent(key_2_3);
    k.SetPosition(19.938, -0.950, 5.249);
    k.t().ry = -5;
  });

  key_2_5.Configure([&](Key& k) {
    k.name = "g";

    // Absolute:
    // k.SetPosition(60.16, 48.06, 37.39);
    // k.t().ry = -30;

    k.SetParent(key_2_4);
    k.SetPosition(20, -1.310, 3.305);
    k.t().ry = -4;
  });

  key_2_2.Configure([&](Key& k) {
    k.name = "s";

    // Absolute:
    // k.SetPosition(6.09, 50.23, 18.05);
    // k.t().ry = -10;

    k.SetParent(key_2_3);
    k.SetPosition(-19.571, -0.090, 5.430);
    k.t().ry = 5;
  });

  key_2_1.Configure([&](Key& k) {
    k.name = "a";

    // Absolute:
    // k.SetPosition(-15.41, 44.06, 19.7);
    // k.t().ry = -10;

    k.SetParent(key_2_2);
    k.SetPosition(-20.887, -6.170, 5.358);
  });

  key_2_0.Configure([&](Key& k) {
    k.name = "caps";

    // Absolute:
    // k.SetPosition(-37.7, 48.06, 15.98);
    // k.t().ry = -5;

    k.SetParent(key_2_1);
    //k.SetPosition(-22.597, 4.000, 0.207);
    k.SetPosition(-20.887, 0, 0);

    //k.t().ry = 5;
  });

  // D Column
  key_1_3 = GetXAxisRotatedKey(kColumn3Radius, rotationDirectionUp);
  key_1_3.Configure([&](Key& k) {
    k.name = "e";
    k.SetParent(key_2_3);
  });

  // This key is different from the others in the column. It should be less angled due to the larger
  // radius.
  key_0_3 = GetXAxisRotatedKey(kColumn3Radius + 15, rotationDirectionUp);
  key_0_3.Configure([&](Key& k) {
    k.name = "3";
    k.SetParent(key_1_3);
  });

  key_3_3 = GetXAxisRotatedKey(kColumn3Radius, rotationDirectionDown);
  key_3_3.Configure([&](Key& k) {
    k.name = "c";
    k.SetParent(key_2_3);
  });

  key_4_3 = GetXAxisRotatedKey(kColumn3Radius, rotationDirectionDown);
  key_4_3.Configure([&](Key& k) {
    k.name = "left_arrow";
    k.SetParent(key_3_3);
  });

  // S column
  key_1_2 = GetXAxisRotatedKey(kColumn2Radius, rotationDirectionUp);
  key_1_2.Configure([&](Key& k) {
    k.name = "w";
    k.SetParent(key_2_2);
  });

  key_0_2 = GetXAxisRotatedKey(kColumn2Radius, rotationDirectionUp);
  key_0_2.Configure([&](Key& k) {
    k.name = "2";
    k.SetParent(key_1_2);
  });

  key_3_2 = GetXAxisRotatedKey(kColumn2Radius, rotationDirectionDown);
  key_3_2.Configure([&](Key& k) {
    k.name = "x";
    k.SetParent(key_2_2);
  });

  key_4_2 = GetXAxisRotatedKey(kColumn2Radius, rotationDirectionDown);
  key_4_2.Configure([&](Key& k) {
    k.name = "slash";
    k.SetParent(key_3_2);
  });

  // F column
  key_1_4 = GetXAxisRotatedKey(kColumn4Radius, rotationDirectionUp);
  key_1_4.Configure([&](Key& k) {
    k.name = "r";
    k.SetParent(key_2_4);
  });

  key_0_4 = GetXAxisRotatedKey(kColumn4Radius, rotationDirectionUp);
  key_0_4.Configure([&](Key& k) {
    k.name = "4";
    k.SetParent(key_1_4);
  });

  key_3_4 = GetXAxisRotatedKey(kColumn4Radius, rotationDirectionDown);
  key_3_4.Configure([&](Key& k) {
    k.name = "v";
    k.SetParent(key_2_4);
  });

  key_4_4 = GetXAxisRotatedKey(kColumn4Radius, rotationDirectionDown);
  key_4_4.Configure([&](Key& k) {
    k.name = "right_arrow";
    k.SetParent(key_3_4);
  });

  key_1_5 = GetXAxisRotatedKey(kColumn5Radius, rotationDirectionUp);
  key_1_5.Configure([&](Key& k) {
    k.name = "t";
    k.SetParent(key_2_5);
  });

  key_0_5 = GetXAxisRotatedKey(kColumn5Radius, rotationDirectionUp);
  key_0_5.Configure([&](Key& k) {
    k.name = "5";
    k.SetParent(key_1_5);
  });

  key_3_5 = GetXAxisRotatedKey(kColumn5Radius, rotationDirectionDown);
  key_3_5.Configure([&](Key& k) {
    k.name = "b";
    k.SetParent(key_2_5);
  });

  // A column
  key_1_1 = GetXAxisRotatedKey(kColumn1Radius, rotationDirectionUp);
  key_1_1.Configure([&](Key& k) {
    k.name = "q";
    k.SetParent(key_2_1);
  });

  key_0_1 = GetXAxisRotatedKey(kColumn1Radius, rotationDirectionUp);
  key_0_1.Configure([&](Key& k) {
    k.name = "1";
    k.SetParent(key_1_1);
  });

  key_3_1 = GetXAxisRotatedKey(kColumn1Radius, rotationDirectionDown);
  key_3_1.Configure([&](Key& k) {
    k.name = "z";
    k.SetParent(key_2_1);
  });

  key_4_1 = GetXAxisRotatedKey(kColumn1Radius, rotationDirectionDown);
  key_4_1.Configure([&](Key& k) {
    k.name = "tilde";
    k.SetParent(key_3_1);
  });

  // Caps column
  key_1_0 = GetXAxisRotatedKey(kColumn0Radius, rotationDirectionUp);
  key_1_0.Configure([&](Key& k) {
    k.name = "tab";
    k.SetParent(key_2_0);
  });

  key_0_0 = GetXAxisRotatedKey(kColumn0Radius, rotationDirectionUp);
  key_0_0.Configure([&](Key& k) {
    k.name = "plus";
    k.SetParent(key_1_0);
  });

  key_3_0 = GetXAxisRotatedKey(kColumn0Radius, rotationDirectionDown);
  key_3_0.Configure([&](Key& k) {
    k.name = "shift";
    k.SetParent(key_2_0);
  });

  // Keys are measured from the tip of the switch and by default keys are measured from the
  // tip of the cap. Adjust the keys position so that the origin is at the switch top.
  double switch_top_z_offset = 10;
  for (Key* key : all_keys()) {
    key->AddTransform();
    key->disable_switch_z_offset = true;
    key->t().z -= switch_top_z_offset;
  }
}

}  // namespace scad

