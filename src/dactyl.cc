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

// print every step of the way to validate changes
constexpr bool kCreateIntermediateArtifacts = true;

enum class Direction { UP, DOWN, LEFT, RIGHT };

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

void AddShapes(std::vector<Shape>* shapes, std::vector<Shape> to_add) {
  for (Shape s : to_add) {
    shapes->push_back(s);
  }
}

Shape ConnectBowlKeysInternal(KeyData& data);
Shape ConnectBowlKeysGridToWall(KeyData& data);
Shape ConnectBowlKeysWallPosts(KeyData& data);

std::vector<WallPoint> CreateWallPoints(KeyData& data);

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
    UnionAll(test_shapes).WriteToFile("validate_00_test_keys.scad");
    return 0;
  }

  std::vector<Shape> shapes;

  // Contruct the bowl

  // adding the keys for bowl only

  double default_padding = 4;

  // left padding to bowl
  for (Key* key : data.grid.column(0)) {
    if (key) {
      key->extra_width_left = default_padding;
    }
  }

  // right padding to bowl
  for (Key* key : data.grid.column(data.grid.num_columns() - 1)) {
    if (key) {
      key->extra_width_right = default_padding;
    }
  }

  // top padding to bowl
  for (Key* key : data.grid.row(0)) {
    if (key) {
      key->extra_width_top = default_padding;
    }
  }

  // bottom padding to bowl
  for (Key* key : data.grid.row(data.grid.num_rows() - 1)) {
      if (key) {
      key->extra_width_bottom = default_padding;
	}
  }

  // All changes to `data` need to be done before calling the next steps.
  

  // Create the Switch Drop Ins which takes into account the padding. 
  for (Key* key : data.bowl_keys()) {
    shapes.push_back(key->GetSwitch());
    if (kAddCaps) {
      shapes.push_back(key->GetCap().Color("red"));
    }
  }

  // Printing Intermediate Steps
  if (kCreateIntermediateArtifacts) {
    UnionAll(shapes).WriteToFile("validate_01_BowlKeysOnly.scad");
  }

  // Creating the grid
  shapes.push_back(ConnectBowlKeysInternal(data));

  // Printing Intermediate Steps
  if (kCreateIntermediateArtifacts) {
    UnionAll(shapes).WriteToFile("validate_02_BowlKeysAndGrid.scad");
  }

  shapes.push_back(ConnectBowlKeysGridToWall(data));

  // Printing Intermediate Steps
  if (kCreateIntermediateArtifacts) {
    UnionAll(shapes).WriteToFile("validate_03_BowlAndPadding.scad");
  }

  shapes.push_back(ConnectBowlKeysWallPosts(data));

  // Printing Intermediate Steps
  if (kCreateIntermediateArtifacts) {
    UnionAll(shapes).WriteToFile("validate_04_BowlAndWalls.scad");
  }

  // adding the keys for thumb cluster
  for (Key* key : data.thumb_keys()) {
    shapes.push_back(key->GetSwitch());
    if (kAddCaps) {
      shapes.push_back(key->GetCap().Color("red"));
    }
  }

  // Set all of the widths here. This must be done before calling any of GetTopLeft etc.

  data.key_thumb_5_0.extra_width_bottom = 11;
  data.key_thumb_5_0.extra_width_left = 3;
  data.key_thumb_5_1.extra_width_bottom = 11;
  data.key_thumb_5_2.extra_width_bottom = 3;
  data.key_thumb_5_5.extra_width_top = 3;
  data.key_thumb_5_4.extra_width_top = 3;
  data.key_thumb_5_4.extra_width_right = 3;
  data.key_thumb_5_4.extra_width_left = 3;
  data.key_thumb_5_3.extra_width_right = 3;
  data.key_thumb_5_3.extra_width_left = 3;
  data.key_thumb_5_3.extra_width_top = 3;
  data.key_thumb_5_2.extra_width_top = 3;
  data.key_thumb_5_2.extra_width_right = 3;
  data.key_thumb_5_2.extra_width_left = 3;


  //
  // Thumb plate
  //

  shapes.push_back(Union(ConnectHorizontal(data.key_thumb_5_5, data.key_thumb_5_4),
                         ConnectHorizontal(data.key_thumb_5_0, data.key_thumb_5_1),
                         ConnectVertical(data.key_thumb_5_5, data.key_thumb_5_1),
                         Tri(data.key_thumb_5_2.GetBottomLeft(),
                             data.key_thumb_5_1.GetBottomRight(),
                             data.key_thumb_5_0.GetBottomLeft())));

  shapes.push_back(TriFan(data.key_thumb_5_5.GetTopLeft(),
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
  shapes.push_back(TriFan(data.key_thumb_5_0.GetBottomLeft(),
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
  shapes.push_back(TriFan(data.key_thumb_5_1.GetTopLeft(),
                          {
                              data.key_thumb_5_5.GetTopLeft(),
                              data.key_3_5.GetBottomRight(),
                              data.key_thumb_5_0.GetTopLeft(),
                          }));
  shapes.push_back(TriFan(data.key_3_5.GetBottomLeft(),
                          {
                              data.key_3_5.GetBottomRight(),
                              data.key_thumb_5_0.GetTopLeft(),
                              data.key_thumb_5_0.GetTopLeft(),
                              data.key_4_4.GetBottomRight(),
                              data.key_4_4.GetTopRight(),
                              data.key_3_4.GetBottomRight(),
                          }));



  std::vector<Shape> negative_shapes;
  //AddShapes(&negative_shapes, screw_holes);
  // Cut off the parts sticking up into the thumb plate.
  negative_shapes.push_back(
      data.key_thumb_5_0.GetTopLeft().Apply(Cube(50, 50, 6).TranslateZ(3)).Color("red"));

  // Cut out hole for holder.
  Shape holder_hole = Cube(29.0, 20.0, 12.5).TranslateZ(12 / 2);
  glm::vec3 holder_location = data.key_0_4.GetTopLeft().Apply(kOrigin);
  holder_location.z = -0.5;
  holder_location.x += 17.5;
  negative_shapes.push_back(holder_hole.Translate(holder_location));

  Shape result = UnionAll(shapes);
  // Subtracting is expensive to preview and is best to disable while testing.
  result = result.Subtract(UnionAll(negative_shapes));
  result.WriteToFile("product_left.scad");
  result.MirrorX().WriteToFile("product_right.scad");

  // Bottom plate
  {
    std::vector<Shape> bottom_plate_shapes = {result};
    for (Key* key : data.all_keys()) {
      bottom_plate_shapes.push_back(Hull(key->GetSwitch()));
    }

    //Shape bottom_plate = UnionAll(bottom_plate_shapes)
    //                         .Projection()
    //                         .LinearExtrude(1.5)
                             //.Subtract(UnionAll(screw_holes));
    //bottom_plate.WriteToFile("product_left_bottom.scad");
    //bottom_plate.MirrorX().WriteToFile("product_right_bottom.scad");
  }

  return 0;
}

Shape ConnectBowlKeysInternal(KeyData& data) {
  std::vector<Shape> shapes;
  for (int r = 0; r < data.grid.num_rows(); ++r) {
    for (int c = 0; c < data.grid.num_columns(); ++c) {
      Key* key = data.grid.get_key(r, c);
      if (!key) {
        // No key at this location.
        continue;
      }
      Key* key_on_left = data.grid.get_key(r, c - 1);
      Key* key_on_top_left = data.grid.get_key(r - 1, c - 1);
      Key* key_on_top = data.grid.get_key(r - 1, c);

      if (key_on_left) {
        shapes.push_back(ConnectHorizontal(*key_on_left, *key));
      }
      if (key_on_top) {
        shapes.push_back(ConnectVertical(*key_on_top, *key));
        if (key_on_left && key_on_top_left) {
          shapes.push_back(ConnectDiagonal(*key_on_top_left, *key_on_top, *key, *key_on_left));
        }
      }
    }
  }
  return UnionAll(shapes);
}


Shape ConnectBowlCornerKeys(KeyData& data, GridCorner corner) {
    std::vector<Shape> shapes;

    auto row = corner.index_row;
    auto col = corner.index_column;

    if (corner.key == NULL)
    {
    auto key_left = data.grid.get_key_located_left(row, col);
    auto key_right = data.grid.get_key_located_right(row, col); 
    auto key_up = data.grid.get_key_located_up(row, col); 
    auto key_down = data.grid.get_key_located_down(row, col); 

    auto key_diagonal_top_left = data.grid.get_key_located_diagonal_top_left(row, col);
    auto key_diagonal_top_right = data.grid.get_key_located_diagonal_top_right(row, col);
    auto key_diagonal_bottom_right = data.grid.get_key_located_diagonal_bottom_right(row, col);
    auto key_diagonal_bottom_left = data.grid.get_key_located_diagonal_bottom_left(row, col);


      // Create a triangle if key is missing

      // switch statement over corner.location 

    switch (corner.location) {
      case CornerLocation::BOTTOM_LEFT:
        shapes.push_back(TriFan(key_up->GetBottomRight(),
                                {
                                    key_diagonal_top_right->GetBottomLeft(),
                                    key_right->GetTopLeft(),
                                    key_right->GetBottomLeft(),
                                    key_up->GetBottomLeft(),
                                }));
        break;
      case CornerLocation::BOTTOM_RIGHT:
        shapes.push_back(TriFan(key_up->GetBottomLeft(),
                                {
                                    key_diagonal_top_left->GetBottomRight(),
                                    key_left->GetTopRight(),
                                    key_left->GetBottomRight(),
                                    key_up->GetBottomRight(),
                                }));
        break;
    }

    }
      return UnionAll(shapes);
}

Shape ConnectBowlKeysGridToWall(KeyData& data) {
  std::vector<Shape> shapes;

  auto corners = data.grid.get_key_corners();

  // TODO: Process all the cornes first.
  shapes.push_back(ConnectBowlCornerKeys(data, corners[2]));
  shapes.push_back(ConnectBowlCornerKeys(data, corners[3]));

  // Top Row
  for (size_t i = 0; i < data.grid.num_columns(); i++) {

    // get the key at the top row and the previous column 
	Key* key_previous = data.grid.get_key(0, i - 1);
	// get the key at the top row and the current column
	Key* key = data.grid.get_key(0, i);
	// get the key at the top row and the next column
    Key* key_next = data.grid.get_key(0, i + 1);
    
	// if the key is not null
    if (key) {
	  // if the previous key to it is not null
        if (key_previous) {
        shapes.push_back(TriFan(key->GetTopLeft().TranslateFront(0, 0, 0), 
                                   {
                                    key->GetTopLeft(),
                                    key_previous->GetTopRight(),
                                    key_previous->GetTopLeft(),
                                   }
        ));
	  }
	}
  }

  // Bottom Row in reverse order
  for (int16_t i = data.grid.num_columns() - 1; i >= 0; i--) {
        size_t lastRowIndex = data.grid.num_rows() - 1;

        // get the key at the top row and the previous column
        Key* key_previous = data.grid.get_key(lastRowIndex, i - 1);
        // get the key at the top row and the current column
        Key* key = data.grid.get_key(lastRowIndex, i);
        // get the key at the top row and the next column
        Key* key_next = data.grid.get_key(lastRowIndex, i + 1);


        if (key) {
          // next here is the same as previously since we are going back  
          if (key_next) {
            shapes.push_back(TriFan(key->GetBottomRight().TranslateFront(0, 0, 0),
                                {
                                    key->GetBottomRight(),
                                    key_next->GetBottomLeft(),
                                    key_next->GetBottomRight(),
                                }));
          }
        }
  }


  return UnionAll(shapes);
}

Shape ConnectBowlKeysWallPosts(KeyData& data) {

  std::vector<Shape> shapes;

  //
  // Make the wall
  //
  {

    std::vector<WallPoint> wall_points = CreateWallPoints(data);

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

    glm::vec3 screw_right_bottom = data.key_thumb_5_2.GetBottomLeft().Apply(kOrigin);
    screw_right_bottom.z = 0;
    screw_right_bottom.y += 3.5;
    screw_right_bottom.x += 1.5;

    glm::vec3 screw_right_mid = data.key_thumb_5_5.GetTopLeft().Apply(kOrigin);
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
  return UnionAll(shapes);
}

std::vector<WallPoint> CreateWallPoints(KeyData& data) {

  Direction direction_top_row_is_up = Direction::UP;
  Direction direction_bottom_row_is_down = Direction::DOWN;
  Direction direction_left_column_is_left = Direction::LEFT;
  Direction direction_right_column_is_right = Direction::RIGHT;

  auto corners = data.grid.get_key_corners();

  // Clock Wise
  GridCorner corner_top_left = corners[0];
  GridCorner corner_top_right = corners[1];
  GridCorner corner_bottom_right = corners[2];
  GridCorner corner_bottom_left = corners[3];


  // Start top left and go clockwise.
  // Top Row: Left to Right
  // Right Column: Top to Bottom
  // Bottom Row: Right to Left
  // Left Column: Bottom to Top
  std::vector<WallPoint> wall_points = {};

  // Top Row left to right
  for (Key* key : data.grid.row(0)) {
    if (key) {
          wall_points.push_back({key->GetTopLeft(), direction_top_row_is_up});
    }
  } 

  // corner_top_right handleling
  if (corner_top_right.key == NULL) {
    auto row = corner_top_right.index_row;
    auto col = corner_top_right.index_column;
    auto key_to_the_bottom = data.grid.get_key_located_down(row, col);
    auto key_to_the_left = data.grid.get_key_located_left(row, col);

    wall_points.push_back({key_to_the_left->GetTopRight(), direction_top_row_is_up, 0, .75});
    wall_points.push_back({key_to_the_left->GetTopRight(), direction_right_column_is_right, 0, .5});
    wall_points.push_back(
        {key_to_the_bottom->GetTopRight(), direction_right_column_is_right, 0, .5});
  }

  if (corner_top_right.key) {
    auto currentKey = corner_top_right.key;
    // removing the sharp corner
    wall_points.push_back({currentKey->GetTopRight(), direction_top_row_is_up, 0, .5});
    wall_points.push_back({currentKey->GetTopRight(), direction_right_column_is_right, 0, .5});
  }

 // Right column, top to bottom
 for (size_t i = 0; i < data.grid.num_rows() -1; i++) {
    Key* key = data.grid.get_key(i, data.grid.num_columns() - 1);
    if (key) {
          wall_points.push_back({key->GetTopRight(), direction_right_column_is_right});
    }
  }

  // corner_bottom_right handleling
  if (corner_bottom_right.key == NULL) {
    auto row = corner_bottom_right.index_row;
    auto col = corner_bottom_right.index_column;
    auto key_to_the_top = data.grid.get_key_located_up(row, col);
    auto key_to_the_left = data.grid.get_key_located_left(row, col);

    wall_points.push_back(
        {key_to_the_top->GetBottomRight(), direction_right_column_is_right, 0, .75});
    wall_points.push_back(
        {key_to_the_top->GetBottomRight(), direction_bottom_row_is_down, 0, .5});
    wall_points.push_back({key_to_the_left->GetBottomRight(), direction_bottom_row_is_down, 0, .5});
  }

  if (corner_bottom_right.key) {
    auto currentKey = corner_bottom_right.key;
    // removing the sharp corner
    wall_points.push_back({currentKey->GetBottomRight(), direction_right_column_is_right, 0, .5});
    wall_points.push_back({currentKey->GetBottomRight(), direction_bottom_row_is_down, 0, .5});
  }


 // Bottom row, right to left. iterate a column in reverse order
 for (int16_t i = data.grid.num_columns() - 1; i >= 0; i--) {
    Key* key = data.grid.get_key(data.grid.num_rows() - 1, i);
    if (key) {
          wall_points.push_back({key->GetBottomRight(), direction_bottom_row_is_down});
    }
  }

 //corner_bottom_left handleling
  if (corner_bottom_left.key == NULL) {
    auto row = corner_bottom_left.index_row;
    auto col = corner_bottom_left.index_column;
    auto key_to_the_top = data.grid.get_key_located_up(row, col);
    auto key_to_the_right = data.grid.get_key_located_right(row, col);

    wall_points.push_back(
        {key_to_the_right->GetBottomLeft(), direction_bottom_row_is_down, 0, .75});
    wall_points.push_back(
        {key_to_the_right->GetBottomLeft(), direction_left_column_is_left, 0, .5});
    wall_points.push_back({key_to_the_top->GetBottomLeft(), direction_left_column_is_left, 0, .5});
  }
  if (corner_bottom_left.key)
  {
    auto currentKey = corner_bottom_left.key;
    // removing the sharp corner
    wall_points.push_back({currentKey->GetBottomLeft(), direction_bottom_row_is_down, 0, .5});
    wall_points.push_back({currentKey->GetBottomLeft(), direction_left_column_is_left, 0, .5});
  }

  // Left Column, bottom to top
  for (int16_t i = data.grid.num_rows() - 1; i >= 0; i--) {
    Key* key = data.grid.get_key(i, 0);
    if (key) {
          wall_points.push_back({key->GetBottomLeft(), direction_left_column_is_left});
    }
  }

   // corner_top_left handleling
  if (corner_top_left.key == NULL) {
    auto row = corner_top_left.index_row;
    auto col = corner_top_left.index_column;
    auto key_to_the_bottom = data.grid.get_key_located_down(row, col);
    auto key_to_the_right = data.grid.get_key_located_right(row, col);

    wall_points.push_back({key_to_the_bottom->GetTopLeft(), direction_left_column_is_left, 0, .75});
    wall_points.push_back({key_to_the_bottom->GetTopLeft(), direction_top_row_is_up, 0, .5});
    wall_points.push_back({key_to_the_right->GetTopLeft(), direction_top_row_is_up, 0, .5});
  } 
  
  if (corner_top_left.key)
  {
    auto currentKey = corner_top_left.key;
    // removing the sharp corner
    wall_points.push_back({currentKey->GetTopLeft(), direction_left_column_is_left, 0, .5});
    wall_points.push_back({currentKey->GetTopLeft(), direction_top_row_is_up, 0, .5});
  }

  return wall_points;
}
