#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "key.h"
#include "key_data.h"
#include "scad.h"
#include "transform.h"

using namespace scad;

constexpr bool kWriteTestKeys = false;
// Add the caps into the stl for testing.
constexpr bool kAddCaps = false;

enum class Direction { UP, DOWN, LEFT, RIGHT };

void AddShapes(std::vector<Shape>* shapes, std::vector<Shape> to_add) {
  for (Shape s : to_add) {
    shapes->push_back(s);
  }
}

Shape ConnectMainKeys(KeyData& d);

int main() {
  printf("generating..\n");
  TransformList key_origin;
  key_origin.Translate(-20, -40, 3);

  // This is where all of the logic to position the keys is done. Everything below is cosmetic
  // trying to build the case.
  KeyData data(key_origin);

  if (kWriteTestKeys) {
    std::vector<Shape> test_shapes;
    std::vector<Key*> test_keys = {&data.key_0_3, &data.key_1_3, &data.key_0_4, &data.key_0_5, &data.key_2_3};
    for (Key* key : test_keys) {
      key->add_side_nub = false;
      key->extra_z = 4;
      test_shapes.push_back(key->GetSwitch());
      if (kAddCaps) {
        test_shapes.push_back(key->GetCap().Color("red"));
      }
    }
    UnionAll(test_shapes).WriteToFile("test_keys.scad");
    return 0;
  }

  // Set all of the widths here. This must be done before calling any of GetTopLeft etc.

  data.key_backspace.extra_width_bottom = 11;
  data.key_backspace.extra_width_left = 3;
  data.key_delete.extra_width_bottom = 11;
  data.key_end.extra_width_bottom = 3;
  data.key_ctrl.extra_width_top = 3;
  data.key_alt.extra_width_top = 3;
  data.key_alt.extra_width_right = 3;
  data.key_alt.extra_width_left = 3;
  data.key_home.extra_width_right = 3;
  data.key_home.extra_width_left = 3;
  data.key_home.extra_width_top = 3;
  data.key_end.extra_width_top = 3;
  data.key_end.extra_width_right = 3;
  data.key_end.extra_width_left = 3;

  // left wall
  for (Key* key : data.grid.column(0)) {
    if (key) {
      key->extra_width_left = 4;
    }
  }

  data.key_0_5.extra_width_right = 4;
  data.key_1_5.extra_width_right = 4;
  data.key_2_5.extra_width_right = 4;

  for (Key* key : data.grid.row(0)) {
    // top row
    if (key) {
      key->extra_width_top = 2;
    }
  }
  data.key_3_5.extra_width_bottom = 3;

  std::vector<Shape> shapes;

  //
  // Thumb plate
  //

  shapes.push_back(Union(ConnectHorizontal(data.key_ctrl, data.key_alt),
                         ConnectHorizontal(data.key_backspace, data.key_delete),
                         ConnectVertical(data.key_ctrl, data.key_delete),
                         Tri(data.key_end.GetBottomLeft(),
                             data.key_delete.GetBottomRight(),
                             data.key_backspace.GetBottomLeft())));

  shapes.push_back(ConnectMainKeys(data));

  shapes.push_back(TriFan(data.key_ctrl.GetTopLeft(),
                          {
                              data.key_3_5.GetBottomRight(),
                              data.key_3_5.GetTopRight(),
                              data.key_2_5.GetBottomRight(),
                          })

  );

  // These transforms with TranslateFront are moving the connectors down in the z direction to
  // reduce the vertical jumps.
  TransformList slash_bottom_right = data.key_4_2.GetBottomRight().TranslateFront(0, -5, -3);

  shapes.push_back(TriFan(slash_bottom_right,
                          {
                              data.key_4_3.GetBottomRight().TranslateFront(0, 0, -1),
                              data.key_4_3.GetBottomLeft(),
                              data.key_4_2.GetBottomRight().TranslateFront(0, 0, -1),
                          }));
  shapes.push_back(TriFan(data.key_backspace.GetBottomLeft(),
                          {
                              slash_bottom_right,
                              data.key_4_3.GetBottomRight().TranslateFront(0, 0, -1),
                              data.key_4_4.GetBottomLeft().TranslateFront(0, 0, -1),
                              data.key_4_4.GetBottomRight(),
                          }));
  shapes.push_back(TriFan(data.key_4_1.GetBottomRight(),
                          {
                              data.key_4_2.GetBottomLeft(),
                              data.key_4_2.GetBottomRight().TranslateFront(0, 0, -1),
                              slash_bottom_right,
                          }));
  shapes.push_back(TriFan(data.key_delete.GetTopLeft(),
                          {
                              data.key_ctrl.GetTopLeft(),
                              data.key_3_5.GetBottomRight(),
                              data.key_backspace.GetTopLeft(),
                          }));
  shapes.push_back(TriFan(data.key_3_5.GetBottomLeft(),
                          {
                              data.key_3_5.GetBottomRight(),
                              data.key_backspace.GetTopLeft(),
                              data.key_backspace.GetTopLeft(),
                              data.key_4_4.GetBottomRight(),
                              data.key_4_4.GetTopRight(),
                              data.key_3_4.GetBottomRight(),
                          }));

  // Bottom right corner.
  shapes.push_back(TriFan(data.key_3_0.GetBottomRight(),
                          {
                              data.key_3_1.GetBottomLeft(),
                              data.key_4_1.GetTopLeft(),
                              data.key_4_1.GetBottomLeft(),
                              data.key_3_0.GetBottomLeft(),
                          }));

  // Connecting top wall to keys
  TransformList key_plus_top_right_wall = data.key_0_0.GetTopRight().TranslateFront(0, 3, -3);
  TransformList key_2_top_left_wall = data.key_0_2.GetTopLeft().TranslateFront(0, 3.75, 0);
  TransformList key_2_top_right_wall = data.key_0_2.GetTopRight().TranslateFront(0, 4, -1);
  TransformList key_3_top_right_wall = data.key_0_3.GetTopRight().TranslateFront(0, 3.5, 0);
  TransformList key_4_top_right_wall = data.key_0_4.GetTopRight().TranslateFront(0, 2.2, 0);

  shapes.push_back(TriFan(key_4_top_right_wall,
                          {
                              data.key_0_5.GetTopRight(),
                              data.key_0_5.GetTopLeft(),
                              data.key_0_4.GetTopRight(),
                              data.key_0_4.GetTopLeft(),
                          }));
  shapes.push_back(TriFan(key_3_top_right_wall,
                          {
                              key_4_top_right_wall,
                              data.key_0_4.GetTopLeft(),
                              data.key_0_3.GetTopRight(),
                              data.key_0_3.GetTopLeft(),
                              key_2_top_right_wall,
                          }));
  shapes.push_back(TriFan(key_2_top_right_wall,
                          {
                              key_2_top_left_wall,
                              data.key_0_2.GetTopRight(),
                              data.key_0_3.GetTopLeft(),
                          }));
  shapes.push_back(TriFan(key_2_top_left_wall,
                          {
                              data.key_0_1.GetTopRight(),
                              data.key_0_2.GetTopLeft(),
                              data.key_0_2.GetTopRight(),
                          }));
  shapes.push_back(TriFan(data.key_0_0.GetTopRight(),
                          {
                              data.key_0_1.GetTopLeft(),
                              data.key_0_1.GetTopRight(),
                              key_2_top_left_wall,
                          }));
  shapes.push_back(TriFan(key_plus_top_right_wall,
                          {
                              key_2_top_left_wall,
                              data.key_0_0.GetTopRight(),
                              data.key_0_0.GetTopLeft(),
                          }));
  //
  // Make the wall
  //
  {
    struct WallPoint {
      WallPoint(TransformList transforms,
                Direction out_direction,
                float extra_distance = 0,
                float extra_width = 0)
          : transforms(transforms),
            out_direction(out_direction),
            extra_distance(extra_distance),
            extra_width(extra_width) {
      }
      TransformList transforms;
      Direction out_direction;
      float extra_distance;
      float extra_width;
    };

    Direction up = Direction::UP;
    Direction down = Direction::DOWN;
    Direction left = Direction::LEFT;
    Direction right = Direction::RIGHT;

    std::vector<WallPoint> wall_points = {
        // Start top left and go clockwise
        {data.key_0_0.GetTopLeft(), up},
        {key_plus_top_right_wall, up, 0, .3},

        {key_2_top_left_wall, up, 0, .3},
        {key_2_top_right_wall, up},

        //{d.key_3.GetTopLeft(), up},
        {key_3_top_right_wall, up},

        // {d.key_4.GetTopLeft(), up},
        {key_4_top_right_wall, up},
        {data.key_0_5.GetTopRight(), up},
        {data.key_0_5.GetTopRight(), right},
        {data.key_0_5.GetBottomRight(), right},

        {data.key_1_5.GetTopRight(), right},
        {data.key_1_5.GetBottomRight(), right},

        {data.key_2_5.GetTopRight(), right},
        {data.key_2_5.GetBottomRight(), right, 1, .5},

        {data.key_ctrl.GetTopLeft().RotateFront(0, 0, -15), up, 1, .5},
        {data.key_ctrl.GetTopRight(), up},

        {data.key_alt.GetTopLeft(), up},
        {data.key_alt.GetTopRight(), up, 0, .5},
        {data.key_alt.GetTopRight(), right, 0, .5},
        {data.key_alt.GetBottomRight(), right},

        {data.key_home.GetTopRight(), right},
        {data.key_home.GetBottomRight(), right},

        {data.key_end.GetTopRight(), right},
        {data.key_end.GetBottomRight(), right, 0, .5},
        {data.key_end.GetBottomRight(), down, 0, .5},
        {data.key_end.GetBottomLeft(), down},

        {data.key_backspace.GetBottomLeft(), down},

        {slash_bottom_right, down},

        {data.key_4_1.GetBottomRight(), down},
        {data.key_4_1.GetBottomLeft(), down},

        {data.key_3_0.GetBottomLeft(), down, 0, .75},
        {data.key_3_0.GetBottomLeft(), left, 0, .5},
        {data.key_3_0.GetTopLeft(), left, 0, .5},

        {data.key_2_0.GetBottomLeft(), left},
        {data.key_2_0.GetTopLeft(), left},

        {data.key_1_0.GetBottomLeft(), left},
        {data.key_1_0.GetTopLeft(), left},

        {data.key_0_0.GetBottomLeft(), left},
        {data.key_0_0.GetTopLeft(), left},
    };

    std::vector<std::vector<Shape>> wall_slices;
    for (WallPoint point : wall_points) {
      Shape s1 = point.transforms.Apply(GetPostConnector());

      TransformList t = point.transforms;
      glm::vec3 out_dir;
      float distance = 4.8 + point.extra_distance;
      switch (point.out_direction) {
        case Direction::UP:
          t.AppendFront(TransformList().Translate(0, distance, 0).RotateX(-20));
          break;
        case Direction::DOWN:
          t.AppendFront(TransformList().Translate(0, -1 * distance, 0).RotateX(20));
          break;
        case Direction::LEFT:
          t.AppendFront(TransformList().Translate(-1 * distance, 0, 0).RotateY(-20));
          break;
        case Direction::RIGHT:
          t.AppendFront(TransformList().Translate(distance, 0, 0).RotateY(20));
          break;
      }

      // Make sure the section extruded to the bottom is thick enough. With certain angles the
      // projection is very small if you just use the post connector from the transform. Compute
      // an explicit shape.
      const glm::vec3 post_offset(0, 0, -4);
      const glm::vec3 p = point.transforms.Apply(post_offset);
      const glm::vec3 p2 = t.Apply(post_offset);

      glm::vec3 out_v = p2 - p;
      out_v.z = 0;
      const glm::vec3 in_v = -1.f * glm::normalize(out_v);

      float width = 3.3 + point.extra_width;
      Shape s2 = Hull(Cube(.1).Translate(p2), Cube(.1).Translate(p2 + (width * in_v)));

      std::vector<Shape> slice;
      slice.push_back(Hull(s1, s2));
      slice.push_back(Hull(s2, s2.Projection().LinearExtrude(.1).TranslateZ(.05)));

      wall_slices.push_back(slice);
    }

    for (size_t i = 0; i < wall_slices.size(); ++i) {
      auto& slice = wall_slices[i];
      auto& next_slice = wall_slices[(i + 1) % wall_slices.size()];
      for (size_t j = 0; j < slice.size(); ++j) {
        shapes.push_back(Hull(slice[j], next_slice[j]));
        // Uncomment for testing. Much faster and easier to visualize.
        // shapes.push_back(slice[j]);
      }
    }
  }

  for (Key* key : data.all_keys()) {
    shapes.push_back(key->GetSwitch());
    if (kAddCaps) {
      shapes.push_back(key->GetCap().Color("red"));
    }
  }

  // Add all the screw inserts.
  std::vector<Shape> screw_holes;
  {
    double screw_height = 5;
    double screw_radius = 4.4 / 2.0;
    Shape screw_hole = Cylinder(screw_height + 2, screw_radius, 30);
    Shape screw_insert =
        Cylinder(screw_height, screw_radius + 1.65, 30).TranslateZ(screw_height / 2);

    glm::vec3 screw_left_bottom = data.key_3_0.GetBottomLeft().Apply(kOrigin);
    screw_left_bottom.z = 0;
    screw_left_bottom.x += 3.2;

    glm::vec3 screw_left_top = data.key_0_0.GetTopLeft().Apply(kOrigin);
    screw_left_top.z = 0;
    screw_left_top.x += 2.8;
    screw_left_top.y += -.5;

    glm::vec3 screw_right_top = data.key_0_5.GetTopRight().Apply(kOrigin);
    screw_right_top.z = 0;
    screw_right_top.x += 4;
    screw_right_top.y += -15.5;

    glm::vec3 screw_right_bottom = data.key_end.GetBottomLeft().Apply(kOrigin);
    screw_right_bottom.z = 0;
    screw_right_bottom.y += 3.5;
    screw_right_bottom.x += 1.5;

    glm::vec3 screw_right_mid = data.key_ctrl.GetTopLeft().Apply(kOrigin);
    screw_right_mid.z = 0;
    screw_right_mid.y += -.9;

    shapes.push_back(Union(screw_insert.Translate(screw_left_top),
                           screw_insert.Translate(screw_right_top),
                           screw_insert.Translate(screw_right_mid),
                           screw_insert.Translate(screw_right_bottom),
                           screw_insert.Translate(screw_left_bottom)));
    screw_holes = {
        screw_hole.Translate(screw_left_top),
        screw_hole.Translate(screw_right_top),
        screw_hole.Translate(screw_right_mid),
        screw_hole.Translate(screw_right_bottom),
        screw_hole.Translate(screw_left_bottom),
    };
  }

  std::vector<Shape> negative_shapes;
  AddShapes(&negative_shapes, screw_holes);
  // Cut off the parts sticking up into the thumb plate.
  negative_shapes.push_back(
      data.key_backspace.GetTopLeft().Apply(Cube(50, 50, 6).TranslateZ(3)).Color("red"));

  // Cut out hole for holder.
  Shape holder_hole = Cube(29.0, 20.0, 12.5).TranslateZ(12 / 2);
  glm::vec3 holder_location = data.key_0_4.GetTopLeft().Apply(kOrigin);
  holder_location.z = -0.5;
  holder_location.x += 17.5;
  negative_shapes.push_back(holder_hole.Translate(holder_location));

  Shape result = UnionAll(shapes);
  // Subtracting is expensive to preview and is best to disable while testing.
  result = result.Subtract(UnionAll(negative_shapes));
  result.WriteToFile("left.scad");
  result.MirrorX().WriteToFile("right.scad");

  // Bottom plate
  {
    std::vector<Shape> bottom_plate_shapes = {result};
    for (Key* key : data.all_keys()) {
      bottom_plate_shapes.push_back(Hull(key->GetSwitch()));
    }

    Shape bottom_plate = UnionAll(bottom_plate_shapes)
                             .Projection()
                             .LinearExtrude(1.5)
                             .Subtract(UnionAll(screw_holes));
    bottom_plate.WriteToFile("bottom_left.scad");
    bottom_plate.MirrorX().WriteToFile("bottom_right.scad");
  }

  return 0;
}

Shape ConnectMainKeys(KeyData& d) {
  std::vector<Shape> shapes;
  for (int r = 0; r < d.grid.num_rows(); ++r) {
    for (int c = 0; c < d.grid.num_columns(); ++c) {
      Key* key = d.grid.get_key(r, c);
      if (!key) {
        // No key at this location.
        continue;
      }
      Key* left = d.grid.get_key(r, c - 1);
      Key* top_left = d.grid.get_key(r - 1, c - 1);
      Key* top = d.grid.get_key(r - 1, c);

      if (left) {
        shapes.push_back(ConnectHorizontal(*left, *key));
      }
      if (top) {
        shapes.push_back(ConnectVertical(*top, *key));
        if (left && top_left) {
          shapes.push_back(ConnectDiagonal(*top_left, *top, *key, *left));
        }
      }
    }
  }
  return UnionAll(shapes);
}
