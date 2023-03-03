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
constexpr bool kAddCaps = true;

constexpr bool isDefaultDactlyThumbCluster = true;

// print every step of the way to validate changes
constexpr bool kCreateIntermediateArtifacts = true;

// TODO: Move this to a better place
std::string connect_point_1_source_bowl = "NOT_SET";
std::string connect_point_1_destination_thumb = "NOT_SET";
std::string connect_point_2_source_thumb = "NOT_SET";
std::string connect_point_2_destination_bowl = "NOT_SET";

enum class Direction { UP, DOWN, LEFT, RIGHT };

struct WallPoint {
  WallPoint(TransformList transforms,
            Direction out_direction,
            float extra_distance = 0,
            float extra_width = 0, 
            std::string unique_id = ""
      )
      : transforms(transforms),
        out_direction(out_direction),
        extra_distance(extra_distance),
        extra_width(extra_width), 
        unique_id(unique_id){
  }
  TransformList transforms;
  Direction out_direction;
  float extra_distance;
  float extra_width;
  std::string unique_id;
};

std::vector<WallPoint> plate_screw_locations;


void AddShapes(std::vector<Shape>* shapes, std::vector<Shape> to_add) {
  for (Shape s : to_add) {
    shapes->push_back(s);
  }
}

Shape ConnectBowlKeysInternalStructure(KeyData& data);
Shape ConnectBowlKeysGridToWall(KeyData& data);
Shape ConnectBowlKeysAndThumbClusterWallPosts(KeyData& data,
                                              bool isDefaultDactlyThumbCluster);
Shape ConnectThumbCluster(KeyData& data, bool isDefaultDactlyThumbCluster);
Shape ConnectCreateWalls(std::vector<WallPoint> wall_points);
std::vector<WallPoint> CreateWallPointsForBowlKeys(KeyData& data);
std::vector<WallPoint> CreateWallPointsForBowlThumbCluster(KeyData& data,
                                                           bool isDefaultDactlyThumbCluster);

int main() {
  printf("generating..\n");
  TransformList key_origin;

  double adjust_height_for_origin = 3;

  // RotateY(15) makes the bowl anchor key to have zero ration but it causes the thumb cluster to gain the angle
  // A RotateY(5) has an effective angle of -10 (5 - 15)  
  // Depending on the y rotation you may need to adjust the starting heigh
  key_origin.Translate(-20, -40, 3 + adjust_height_for_origin); //.RotateY(0);

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
      key->extra_width_top = default_padding + 1;
    }
  }


  // bottom padding to bowl
  for (Key* key : data.grid.row(data.grid.num_rows() - 1)) {
      if (key) {
      key->extra_width_bottom = default_padding + 1;
	}
  }

// removing the extra padding from the key left of the bottom right corner.
  auto key_left_of_corner_bottom_right =
      data.grid.get_key_located_left(data.grid.get_key_corner_bottom_right());
  
  if (key_left_of_corner_bottom_right) {
        key_left_of_corner_bottom_right->extra_width_bottom = 0;
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
  shapes.push_back(ConnectBowlKeysInternalStructure(data));

  // Printing Intermediate Steps
  if (kCreateIntermediateArtifacts) {
    UnionAll(shapes).WriteToFile("validate_02_BowlKeysAndGrid.scad");
  }

  shapes.push_back(ConnectBowlKeysGridToWall(data));

  // Printing Intermediate Steps
  if (kCreateIntermediateArtifacts) {
    UnionAll(shapes).WriteToFile("validate_03_BowlAndPadding.scad");
  }

  // Thumb Cluster
  shapes.push_back(ConnectThumbCluster(data, isDefaultDactlyThumbCluster));

    // Printing Intermediate Steps
  if (kCreateIntermediateArtifacts) {
    UnionAll(shapes).WriteToFile("validate_04_thumbcluster.scad");
  }

  if (kCreateIntermediateArtifacts) {
    // Doing a substract only for preview
    UnionAll(shapes)
        .Subtract(
            data.key_thumb_0_0.GetTopLeft().Apply(Cube(50, 50, 6).TranslateZ(3)).Color("green"))
        .WriteToFile("validate_05_substrack_thumb_preview.scad");
  }

  shapes.push_back(ConnectBowlKeysAndThumbClusterWallPosts(data, isDefaultDactlyThumbCluster));

  // Printing Intermediate Steps
  if (kCreateIntermediateArtifacts) {
    UnionAll(shapes).WriteToFile("validate_06_BowlAndWalls.scad");
  }


    // Add all the screw inserts.
  std::vector<Shape> screw_holes;
  {
    double screw_height = 5;
    double screw_radius = 4.4 / 2.0;
    Shape screw_hole = Cylinder(screw_height + 2, screw_radius, 30);
    Shape screw_insert =
        Cylinder(screw_height, screw_radius + 1.65, 30).TranslateZ(screw_height / 2);


    for (WallPoint wall_point : plate_screw_locations){
      glm::vec3 each = wall_point.transforms.Apply(kOrigin);
      each.z = 0;
	  screw_holes.push_back(screw_hole.Translate(each));
	  shapes.push_back(screw_insert.Translate(each));
	}
  }

   // Printing Intermediate Steps
  if (kCreateIntermediateArtifacts) {
        UnionAll(shapes).WriteToFile("validate_07_KeebAndPlateScrews.scad");
  }

  std::vector<Shape> negative_shapes;
  //AddShapes(&negative_shapes, screw_holes);
  // Cut off the parts sticking up into the thumb plate.
  negative_shapes.push_back(
      data.key_thumb_0_0.GetTopLeft().Apply(Cube(70, 70, 6).TranslateZ(3)).Color("red"));

  // Cut out hole for holder.
  Shape holder_hole = Cube(29.0, 20.0, 12.5).TranslateZ(12 / 2);
  glm::vec3 holder_location = data.grid.get_key_located_left(
            data.grid.get_key_corner_top_right()
                          )->GetTopLeft().Apply(kOrigin);
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

    Shape bottom_plate = UnionAll(bottom_plate_shapes).Projection().LinearExtrude(1.5);
                            // .Subtract(UnionAll(screw_holes));
    bottom_plate.WriteToFile("product_left_bottom.scad");
    bottom_plate.MirrorX().WriteToFile("product_right_bottom.scad");
  }

    printf("done generating\n");

  return 0;
}

Shape ConnectThumbCluster(KeyData& data, bool isDefaultDactlyThumbCluster) {

  std::vector<Shape> shapes;

  constexpr double kDefaultKeySpacing = 19;
  constexpr double kDefaultKeyHalfSpacing = 9.5;

  auto key_bowl_edge_up = data.grid.get_key_located_up(data.grid.get_key_corner_bottom_right());
  auto key_bowl_edge_left = data.grid.get_key_located_left(data.grid.get_key_corner_bottom_right());

      // Depending on the rotation. Both x and y need to change to move the keys away from the keywell.
      // the actual distance will be greater since its the combine vector of the two directions. 
      double adjust_thumb_distance = 6;
         // Thumb cluster is indepenently position from bowl keys.
      double anchor_thumb_x = 60 + adjust_thumb_distance;  // 60;
      double anchor_thumb_y = -9.18 - adjust_thumb_distance;  // -9.18;
      double anchor_thumb_z =
          32.83;  // original (42.83) -10 since we are not adding this offset for the keycaps
      double anchor_thumb_rotate_x = -21;
      double anchor_thumb_rotate_y = 12;
      double anchor_thumb_rotate_z = -4.5;

      //
      // Thumb keys
      //

      data.key_thumb_0_0.Configure([&](Key& k) {
        k.name = "key_thumb_0_0";
        k.SetParent(data.origin_for_bowl);
        k.SetPosition(anchor_thumb_x, anchor_thumb_y, anchor_thumb_z);
        k.t().rz = anchor_thumb_rotate_x;
        k.t().rx = anchor_thumb_rotate_y;
        k.t().ry = anchor_thumb_rotate_z;
      });

      // Second thumb key.
      data.key_thumb_0_1.Configure([&](Key& k) {
        k.name = "key_thumb_0_1";
        k.SetParent(data.key_thumb_0_0);
        k.SetPosition(kDefaultKeySpacing, 0, 0);
      });

      // Bottom side key.
      data.key_thumb_0_2.Configure([&](Key& k) {
        k.name = "key_thumb_0_2";
        k.SetParent(data.key_thumb_0_1);
        k.SetPosition(kDefaultKeySpacing, kDefaultKeyHalfSpacing * -1, 0);
      });


      data.key_thumb_0_0.extra_width_bottom = 11;
      data.key_thumb_0_0.extra_width_left = 1;  // original 3
      data.key_thumb_0_1.extra_width_bottom = 11;
      data.key_thumb_0_2.extra_width_bottom = 3;
      data.key_thumb_0_2.extra_width_top = 1;  // original 3
      data.key_thumb_0_2.extra_width_right = 3;
      data.key_thumb_0_2.extra_width_left = 2.5; // original 3


    if (isDefaultDactlyThumbCluster) {

    // Middle side key.
    data.key_thumb_0_3.Configure([&](Key& k) {
      k.name = "key_thumb_0_3";
      k.SetParent(data.key_thumb_0_1);
      k.SetPosition(kDefaultKeySpacing, kDefaultKeyHalfSpacing, 0);
    });

    // Top side key;
    data.key_thumb_0_4.Configure([&](Key& k) {
      k.name = "key_thumb_0_4";
      k.SetParent(data.key_thumb_0_1);
      k.SetPosition(kDefaultKeySpacing, kDefaultKeyHalfSpacing + kDefaultKeySpacing, 0);
    });

    // Top left key.
    data.key_thumb_0_5.Configure([&](Key& k) {
      k.name = "key_thumb_0_5";
      k.SetParent(data.key_thumb_0_1);
      k.SetPosition(0, kDefaultKeyHalfSpacing + kDefaultKeySpacing, 0);
    });

  // Set all of the widths here. This must be done before calling any of GetTopLeft etc.
    
    data.key_thumb_0_5.extra_width_top = 3;
    data.key_thumb_0_4.extra_width_top = 3;
    data.key_thumb_0_4.extra_width_right = 3;
    data.key_thumb_0_4.extra_width_left = 3;
    data.key_thumb_0_3.extra_width_right = 3;
    data.key_thumb_0_3.extra_width_left = 2.5;
    data.key_thumb_0_3.extra_width_top = 3;
  

    shapes.push_back(Union(ConnectHorizontal(data.key_thumb_0_5, data.key_thumb_0_4),
                           ConnectHorizontal(data.key_thumb_0_0, data.key_thumb_0_1),
                           ConnectVertical(data.key_thumb_0_5, data.key_thumb_0_1),
                           Tri(data.key_thumb_0_2.GetBottomLeft(),
                               data.key_thumb_0_1.GetBottomRight(),
                               data.key_thumb_0_0.GetBottomLeft())));

    shapes.push_back(TriFan(data.key_thumb_0_1.GetTopLeft(),
                            {
                                data.key_thumb_0_0.GetTopRight(),
                                data.key_thumb_0_0.GetTopLeft(),
                                data.key_thumb_0_5.GetTopLeft(),
                            }));



  } else {
    // This cluster will mirror more or less the maniform.

    // Middle side key.
    data.key_thumb_0_3.Configure([&](Key& k) {
      k.name = "key_thumb_0_3";
      k.SetParent(data.key_thumb_0_1);
      k.SetPosition(kDefaultKeyHalfSpacing + kDefaultKeySpacing, kDefaultKeyHalfSpacing, 0);
    });

    // Top side key;
    data.key_thumb_0_4.Configure([&](Key& k) {
      k.name = "key_thumb_0_4";
      k.SetParent(data.key_thumb_0_1);
      k.SetPosition(kDefaultKeyHalfSpacing, kDefaultKeySpacing, 0);
    });

    // Top left key.
    data.key_thumb_0_5.Configure([&](Key& k) {
      k.name = "key_thumb_0_5";
      k.SetParent(data.key_thumb_0_1);
      k.SetPosition(-1 * kDefaultKeyHalfSpacing, 10 + kDefaultKeySpacing - 1, 0);
    });


    data.key_thumb_0_5.extra_width_bottom = 10;
    data.key_thumb_0_5.extra_width_top = 4;
    data.key_thumb_0_4.extra_width_top = 4;
    data.key_thumb_0_3.extra_width_top = 4;
    data.key_thumb_0_3.extra_width_right = 4;

    shapes.push_back(Union(ConnectHorizontal(data.key_thumb_0_4, data.key_thumb_0_3),
                           ConnectHorizontal(data.key_thumb_0_0, data.key_thumb_0_1),
                           ConnectVertical(data.key_thumb_0_4, data.key_thumb_0_1)
                           ));

  }


  // adding the keys for thumb cluster
  for (Key* key : data.thumb_keys()) {
      shapes.push_back(key->GetSwitch());
      if (kAddCaps) {
          shapes.push_back(key->GetCap().Color("red"));
      }
  }

  // Printing Intermediate Steps
  if (kCreateIntermediateArtifacts) {
      UnionAll(shapes).WriteToFile("validate_thumbcluster_01_only_keys.scad");
  }

   
    shapes.push_back(TriFan(data.key_thumb_0_0.GetBottomLeft(),
                            {
                                key_bowl_edge_left->GetBottomLeft(),
                                key_bowl_edge_left->GetBottomRight(),
                                data.key_thumb_0_0.GetTopLeft(),
                            }));

    shapes.push_back(TriFan(data.key_thumb_0_5.GetTopLeft(),
                            {
                                key_bowl_edge_up->GetBottomRight(),
                                key_bowl_edge_left->GetBottomRight(),
                                data.key_thumb_0_0.GetTopLeft(),
                            }));
    
    // Printing Intermediate Steps
  if (kCreateIntermediateArtifacts) {
      UnionAll(shapes).WriteToFile("validate_thumbcluster_02_top_connectors.scad");
  }

  return UnionAll(shapes);
}

std::vector<WallPoint> CreateWallPointsForBowlThumbCluster(
    KeyData& data, bool isDefaultDactlyThumbCluster) {
  // Start top left and go clockwise.
  // Top Row: Left to Right
  // Right Column: Top to Bottom
  // Bottom Row: Right to Left
  // Left Column: Bottom to Top
  std::vector<WallPoint> wall_points = {};

  // Ignoring isDefaultDactlyThumbCluster

      std::string corner_key_top_left_point_top_left = data.key_thumb_0_5.name + "_top_left";

      std::string corner_key_bottom_left_point_bottom_left =
          data.key_thumb_0_0.name + "_bottom_left";
      std::string corner_key_bottom_left_point_top_left =
          data.key_thumb_0_0.name + "_top_left";

      wall_points.push_back(
          {data.key_thumb_0_5.GetTopLeft(), Direction::UP, 0, 0, corner_key_top_left_point_top_left});
     
      wall_points.push_back(
          {data.key_thumb_0_5.GetTopRight(), Direction::UP, 0, 0, data.key_thumb_0_5.name});
      wall_points.push_back(
          {data.key_thumb_0_4.GetTopRight(), Direction::UP, 0, .75, data.key_thumb_0_4.name});
      WallPoint thumb_top_right_corner = {
          data.key_thumb_0_4.GetTopRight(), Direction::RIGHT, 0, 0, data.key_thumb_0_4.name};

      wall_points.push_back(thumb_top_right_corner);

      wall_points.push_back(
          {data.key_thumb_0_3.GetTopRight(), Direction::RIGHT, 0, .75, data.key_thumb_0_3.name});

      wall_points.push_back(
           {data.key_thumb_0_3.GetBottomRight(), Direction::RIGHT, 0, .75, data.key_thumb_0_3.name});

      thumb_top_right_corner.transforms.TranslateX(-4).TranslateY(-2);
      plate_screw_locations.push_back(thumb_top_right_corner);

      wall_points.push_back(
          {data.key_thumb_0_2.GetBottomRight(), Direction::RIGHT, 0, .75, data.key_thumb_0_2.name});
      
      WallPoint bottom_right_corner = {
          data.key_thumb_0_2.GetBottomRight(), Direction::DOWN, 0, 0, data.key_thumb_0_2.name};

      wall_points.push_back(bottom_right_corner);

      // Adjusting so screw holes are inside the wall
      bottom_right_corner.transforms.TranslateX(-3).TranslateY(3);
      plate_screw_locations.push_back(bottom_right_corner);

      wall_points.push_back({data.key_thumb_0_0.GetBottomLeft(),
                             Direction::UP,
                             0,
                             0,
                             corner_key_bottom_left_point_bottom_left});
      wall_points.push_back(
          {data.key_thumb_0_0.GetTopLeft(), Direction::UP, 0, 0, corner_key_bottom_left_point_top_left});

      connect_point_1_destination_thumb = corner_key_top_left_point_top_left;
      connect_point_2_source_thumb = corner_key_bottom_left_point_bottom_left;

  return wall_points;
}

Shape ConnectBowlKeysInternalStructure(KeyData& data) {
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
    auto key_top = data.grid.get_key_located_up(row, col); 
    auto key_bottom = data.grid.get_key_located_down(row, col); 

    auto key_diagonal_top_left = data.grid.get_key_located_diagonal_top_left(row, col);
    auto key_diagonal_top_right = data.grid.get_key_located_diagonal_top_right(row, col);
    auto key_diagonal_bottom_right = data.grid.get_key_located_diagonal_bottom_right(row, col);
    auto key_diagonal_bottom_left = data.grid.get_key_located_diagonal_bottom_left(row, col);


      // Create a triangle if key is missing

      // switch statement over corner.location 

    switch (corner.location) {
      case CornerLocation::TOP_LEFT:
        shapes.push_back(TriFan(key_bottom->GetTopRight(),
                                {
                                    key_diagonal_bottom_right->GetTopLeft(),
                                    key_right->GetBottomLeft(),
                                    key_right->GetTopLeft(),
                                    key_bottom->GetTopLeft(),
                                }));
        break;
      case CornerLocation::TOP_RIGHT:
        shapes.push_back(TriFan(key_bottom->GetTopLeft(),
                                {
                                    key_diagonal_bottom_left->GetTopRight(),
                                    key_left->GetBottomRight(),
                                    key_left->GetTopRight(),
                                    key_bottom->GetTopRight(),
                                }));
        break;
      case CornerLocation::BOTTOM_RIGHT:
        shapes.push_back(TriFan(key_top->GetBottomLeft(),
                                {
                                    key_diagonal_top_left->GetBottomRight(),
                                    key_left->GetTopRight(),
                                    key_left->GetBottomRight(),
                                    key_top->GetBottomRight(),
                                }));
        break;
      case CornerLocation::BOTTOM_LEFT:
        shapes.push_back(TriFan(key_top->GetBottomRight(),
                                {
                                    key_diagonal_top_right->GetBottomLeft(),
                                    key_right->GetTopLeft(),
                                    key_right->GetBottomLeft(),
                                    key_top->GetBottomLeft(),
                                }));
        break;
    }

    }
      return UnionAll(shapes);
}

Shape ConnectBowlKeysGridToWall(KeyData& data) {
  std::vector<Shape> shapes;

  // process all corners
  for (auto& each : data.grid.get_key_corners()) {
    shapes.push_back(ConnectBowlCornerKeys(data, each));
  }

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

Shape ConnectCreateWalls(std::vector<WallPoint> wall_points) {

   std::vector<Shape> shapes;

   bool is_only_post_enabled = false;

  //
  // Make the wall
  //
  {
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

              if (is_only_post_enabled)
              {
                 // Much faster and easier to visualize.
                 // This will add the columns so you can see where the walls are connecting.
                 shapes.push_back(slice[j]);
              }
              else
              {
                   shapes.push_back(Hull(slice[j], next_slice[j]));
              }
          }
        }
  }
  return UnionAll(shapes);
}

Shape ConnectBowlKeysAndThumbClusterWallPosts(KeyData& data, bool isDefaultDactlyThumbCluster) {

   std::vector<Shape> shapes;

   std::vector<WallPoint> wall_points_for_bowl = CreateWallPointsForBowlKeys(data);

   std::vector<WallPoint> wall_points_for_thumb_cluster =
       CreateWallPointsForBowlThumbCluster(data, isDefaultDactlyThumbCluster);

   std::vector<WallPoint> combined_wall_points;

   // Iternating the first part of the bowl points
   for (auto& each : wall_points_for_bowl) {
        combined_wall_points.push_back(each);
        if (each.unique_id == connect_point_1_source_bowl) {
          break;
        }
   }

   // Iterating the thumb cluster.
   auto is_included_wall_points_for_thumb_cluster = false;
   for (auto& each : wall_points_for_thumb_cluster) {
        if (each.unique_id == connect_point_1_destination_thumb) {
          is_included_wall_points_for_thumb_cluster = true;
          // Adding plate screw location
          plate_screw_locations.push_back(each);
        }
        if (is_included_wall_points_for_thumb_cluster) {
          combined_wall_points.push_back(each);
        }

        if (each.unique_id == connect_point_2_source_thumb) {
          is_included_wall_points_for_thumb_cluster = false;
        }
   }

   // Iternating the last part of the bowl
    auto is_included_bowl = false;
    for (auto& each : wall_points_for_bowl) {
        if (each.unique_id == connect_point_2_destination_bowl) {
          is_included_bowl = true;
          // Adding plate screw location
          plate_screw_locations.push_back({each.transforms.clone()->TranslateX(-3).TranslateY(-2),
                                           each.out_direction,
                                           each.extra_width,
                                           each.extra_distance,
                                           connect_point_1_destination_thumb});
        }
        if (is_included_bowl) {
            combined_wall_points.push_back(each);
        }
    }

   shapes.push_back(ConnectCreateWalls(combined_wall_points));

  return UnionAll(shapes);
}

std::vector<WallPoint> CreateWallPointsForBowlKeys(KeyData& data) {

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

  bool is_double_post_enabled = true;

  auto connect_point_subfix_top_right = "_top_right";
  auto connect_point_subfix_bottom_right = "_bottom_right";
  auto connect_point_subfix_top_left = "_top_left";
  auto connect_point_subfix_bottom_left = "_bottom_left";

  auto connect_point_subfix_middle_top = "_middle_top";
  auto connect_point_subfix_middle_bottom = "_middle_bottom";
  auto connect_point_subfix_middle_left = "_middle_left";
  auto connect_point_subfix_middle_right = "_middle_right";

  // Setting Up Connections Points for the thumb cluster
  connect_point_1_source_bowl = data.grid
      .get_key_located_up(corner_bottom_right)->name + connect_point_subfix_bottom_right;

  connect_point_2_destination_bowl =
      data.grid.get_key_located_left(corner_bottom_right)
          ->name + connect_point_subfix_bottom_left;

  // Start top left and go clockwise.
  // Top Row: Left to Right
  // Right Column: Top to Bottom
  // Bottom Row: Right to Left
  // Left Column: Bottom to Top
  std::vector<WallPoint> wall_points = {};

  // Top Row left to right
  for (Key* key : data.grid.row(0)) {
    if (key) {
          auto top_left_id = key->name + connect_point_subfix_top_left;

          wall_points.push_back({key->GetTopLeft(), direction_top_row_is_up, 0, 0, top_left_id});          

          if (is_double_post_enabled) {
              wall_points.push_back({key->GetMiddleTop(), direction_top_row_is_up, 0, 0,  key->name + connect_point_subfix_middle_top});
          }
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
    WallPoint corner_top_right_wall_point = {
        currentKey->GetTopRight(), direction_top_row_is_up, 0, .5};
    wall_points.push_back(
        corner_top_right_wall_point);
    wall_points.push_back({currentKey->GetTopRight(), direction_right_column_is_right, 0, .5});

    plate_screw_locations.push_back(corner_top_right_wall_point);

  }

 // Right column, top to bottom
 for (size_t i = 0; i < data.grid.num_rows() -1; i++) {
    Key* key = data.grid.get_key(i, data.grid.num_columns() - 1);
    if (key) {
          auto wall_point_id_top_right = key->name + connect_point_subfix_top_right;
          auto wall_point_id_bottom_right = key->name + connect_point_subfix_bottom_right;

          wall_points.push_back(
              {key->GetTopRight(), direction_right_column_is_right, 0, 0, wall_point_id_top_right});

          if (is_double_post_enabled) {
              wall_points.push_back({key->GetMiddleRight(),
                                     direction_right_column_is_right,
                                     0,
                                     0,
                                     key->name + connect_point_subfix_middle_right});
          }
          
          if (wall_point_id_bottom_right == connect_point_1_source_bowl) {
            wall_points.push_back({key->GetBottomRight(),
                                   direction_right_column_is_right,
                                   0,
                                   0,
                                   wall_point_id_bottom_right});     
          }
          
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
          auto bottom_left_id = key->name + connect_point_subfix_bottom_left;
          wall_points.push_back({key->GetBottomRight(), direction_bottom_row_is_down, 0,0, key->name + connect_point_subfix_bottom_right});
          
          if (is_double_post_enabled) {
            wall_points.push_back({key->GetMiddleBottom(),
                                   direction_bottom_row_is_down,
                                   0,
                                   0,
                                   key->name + connect_point_subfix_middle_bottom});
          }

          if (bottom_left_id == connect_point_2_destination_bowl) {
            wall_points.push_back(
                {key->GetBottomLeft(), direction_bottom_row_is_down, 0, 0, bottom_left_id});
          }
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
    WallPoint corner_bottom_left_wall_point = {
        currentKey->GetBottomLeft(), direction_bottom_row_is_down, 0, .5};
    wall_points.push_back(corner_bottom_left_wall_point);
    wall_points.push_back({currentKey->GetBottomLeft(), direction_left_column_is_left, 0, .5});

    // Adjusting to screw plate locations
    corner_bottom_left_wall_point.transforms.TranslateX(1);
    plate_screw_locations.push_back(corner_bottom_left_wall_point);
  }

  // Left Column, bottom to top
  for (int16_t i = data.grid.num_rows() - 1; i >= 0; i--) {
    Key* key = data.grid.get_key(i, 0);
    if (key) {
          wall_points.push_back(
              {key->GetBottomLeft(), direction_left_column_is_left, 0,0, key->name + connect_point_subfix_bottom_left});
          
          if (is_double_post_enabled) {
            wall_points.push_back({key->GetMiddleLeft(),
                                   direction_left_column_is_left,
                                   0,
                                   0,
                                   key->name + connect_point_subfix_middle_left});          
          }
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
    WallPoint corner_top_left_wall_point = {
        currentKey->GetTopLeft(), direction_left_column_is_left, 0, .5};
    wall_points.push_back(corner_top_left_wall_point);
    wall_points.push_back({currentKey->GetTopLeft(), direction_top_row_is_up, 0, .5});

    corner_top_left_wall_point.transforms.TranslateX(1);
    plate_screw_locations.push_back(corner_top_left_wall_point);
  }

  return wall_points;
}
