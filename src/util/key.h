#pragma once

#include <functional>
#include "../glm/glm.hpp"
#include <memory>
#include <string>

#include "scad.h"
#include "transform.h"

namespace scad {

// All sizes in mm.
const double kSwitchWidth = 14.4;
const double kSwitchThickness = 4;
const double kWallWidth = 2;

const double kDsaHeight = 8;
const double kSaHeight = 12.5;
// This is the height of the taller side on the key. The short side has kSaHeight.
const double kSaEdgeHeight = 13.7;
const double kDsaTopSize = 13.2;     // 0.5 * kMmPerInch;
const double kDsaBottomSize = 18.4;  // 0.725 * kMmPerInch;
// The size half way up the key used to generate the cap shape.
const double kDsaHalfSize = 16.2;
const double kSaHalfSize = 17.2;

// Size for the tall sa edged key.
const double kSaTallHeight = 14;
const double kSaTallEdgeHeight = 16.5;

const double kSwitchHorizontalOffset = kSwitchWidth / 2 + kWallWidth;

// This is the distance between the top of the switch plate and the tip of the switch stem.
const double kSwitchTipOffset = 10;

enum class CornerLocation { TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT };

enum class KeyType {
  DSA,
  SA,            // Row 3
  SA_EDGE,       // Row 2
  SA_TALL_EDGE,  // Row 1
};
// For SA edge variants. Which side of the key the edge should be rendered.
enum class SaEdgeType { LEFT, RIGHT, TOP, BOTTOM };

struct Key {
 public:
  Key() {
  }

  Key(double x, double y, double z) {
    t().x = x;
    t().y = y;
    t().z = z;
  }

  // A name for debugging purposes. Can also be used to determine the names for key specific output
  // files.
  std::string name;

  std::string parent_identifier = "";
  TransformList parent_transforms;
  TransformList local_transforms;

  // These add extra width to the switch and offset the locations of the corners (GetTopLeft etc).
  double extra_width_top = 0;
  double extra_width_bottom = 0;
  double extra_width_left = 0;
  double extra_width_right = 0;

  // This makes the switch taller (moves the corner connectors down and increases wall height) while
  // leaving the top plate of the switch at the same point.
  double extra_z = 0;

  bool add_side_nub = true;
  bool disable_switch_z_offset = false;

  KeyType type = KeyType::DSA;
  SaEdgeType sa_edge_type = SaEdgeType::BOTTOM;

  void Configure(std::function<void(Key& k)> fn) {
    fn(*this);
  }

  Key& SetPosition(double x, double y, double z);
  Key& SetParent(const Key& key);
  Key& SetParent(const TransformList& transforms);

  Transform& t() {
    return local_transforms.mutable_front();
  }

  Transform& AddTransform() {
    return local_transforms.AddTransformFront();
  }

  TransformList GetTransforms() const;
  TransformList GetSwitchTransforms() const;

  void description() const;

  Shape GetSwitch() const;
  Shape GetInverseSwitch() const;
  // Used to subtract and clear space in the key cap's path. Vertical length can be explicitly
  // passed to support cutting out for long keys like enter on the kinesis.
  Shape GetInverseCap(double custom_vertical_length = -1) const;
  Shape GetCap(bool fill_in_cap_path = false) const;

  // This is the outermost conner of the switch. You can specify an offset to scale the point back
  // by the specified x,y amount towards the center of the switch. If you had a centered 2x2 post
  // and you wanted the corner to line up with outmost point of the switch, you could specify an
  // offset of -1.
  TransformList GetTopRight(double offset = 0) const;
  TransformList GetTopLeft(double offset = 0) const;
  TransformList GetBottomRight(double offset = 0) const;
  TransformList GetBottomLeft(double offset = 0) const;
  // The same as GetSwitchTransforms but with a more clear name if you are intending to find the
  // middle point.
  TransformList GetMiddle() const;

  TransformList GetMiddleBottom() const;
  TransformList GetMiddleTop() const;
  TransformList GetMiddleLeft() const;
  TransformList GetMiddleRight() const;

  // Corners clockwise starting at top left. Will have size 4.
  std::vector<TransformList> GetCorners(double offset = 0) const;

 private:
  // These are the inner corners of the switch plate.
  TransformList GetTopRightInternal() const;
  TransformList GetTopLeftInternal() const;
  TransformList GetBottomRightInternal() const;
  TransformList GetBottomLeftInternal() const;

  TransformList GetMiddleBottomInternal() const;
  TransformList GetMiddleTopInternal() const;
  TransformList GetMiddleLeftInternal() const;
  TransformList GetMiddleRightInternal() const;
};


struct GridCorner {
 public:
  GridCorner(Key* key, CornerLocation location, size_t index_row, size_t index_column)
      : key(key), location(location), index_column(index_column), index_row(index_row) {
  }
  Key* key = nullptr;
  CornerLocation location;
  size_t index_row;
  size_t index_column;

  bool isSame(size_t index_row, size_t index_column) {
    return index_row == this->index_row && index_column == this->index_column;
  }
};


struct KeyGrid {
  explicit KeyGrid(std::vector<std::vector<Key*>> data) : data(std::move(data)) {
  }

  std::vector<Key*> column(int c) {
    std::vector<Key*> result;
    for (auto& row : data) {
      result.push_back(row[c]);
    }
    return result;
  }

  std::vector<Key*> row(int r) {
    return data[r];
  }

  Key* get_key(int row, int column) {
    if (row < 0 || row >= num_rows()) {
      return nullptr;
    }
    auto& r = data[row];
    if (column < 0 || column >= r.size()) {
      return nullptr;
    }
    return r[column];
  }

  Key* get_key_located_diagonal_top_right(int row, int column) {
    return get_key(row -1, column + 1);
  }

  Key* get_key_located_diagonal_top_left(int row, int column) {
    return get_key(row - 1, column - 1);
  }

  Key* get_key_located_diagonal_bottom_left(int row, int column) {
    return get_key(row + 1, column - 1);
  }
  
  Key* get_key_located_diagonal_bottom_right(int row, int column) {
    return get_key(row + 1, column + 1);
  }

  Key* get_key_located_up(int row, int column) {
    return get_key(row - 1, column);
  }

  Key* get_key_located_down(int row, int column) {
    return get_key(row + 1, column);
  }
 
  Key* get_key_located_left(int row, int column) {
    return get_key(row, column - 1);
  }

  Key* get_key_located_right(int row, int column) {
    return get_key(row, column + 1);
  }

  // returns keys at the cornes going clockwise starting at top left. Will have size 4
  std::vector<GridCorner> get_key_corners() {
    std::vector<GridCorner> result;

    auto columnLastIndex = num_columns() - 1;
    auto rowLastIndex = num_rows() - 1;

    // top row = 0
    size_t key_top_left_row = 0, key_top_right_row = key_top_left_row;
    // left column = 0
    size_t key_top_left_column = 0, key_bottom_left_column = key_top_left_column;
    // bottom row = rowLastIndex
    size_t key_bottom_left_row = rowLastIndex, key_bottom_right_row = key_bottom_left_row;
    // right column = columnLastIndex
    size_t key_top_right_column = columnLastIndex, key_bottom_right_column = key_top_right_column;

    auto key_top_left_corner = get_key(key_top_left_row, key_top_left_column);
    auto key_bottom_left_corner = get_key(key_bottom_left_row, key_bottom_left_column);
    auto key_top_right_corner = get_key(key_top_right_row, key_top_right_column);
    // In a Dactly this key is alway null on the left half of a keyboard. This is the key next to
    // the thumb cluster.
    auto key_bottom_right_corner =
        get_key(key_bottom_right_row, key_bottom_right_column);

    result.push_back( GridCorner( key_top_left_corner, CornerLocation::TOP_LEFT, key_top_left_row, key_top_left_column)); 
    result.push_back( GridCorner( key_top_right_corner, CornerLocation::TOP_RIGHT, key_top_right_row, key_top_right_column)); 
    result.push_back( GridCorner( key_bottom_right_corner, CornerLocation::BOTTOM_RIGHT, key_bottom_right_row, key_bottom_right_column)); 
    result.push_back( GridCorner( key_bottom_left_corner, CornerLocation::BOTTOM_LEFT, key_bottom_left_row, key_bottom_left_column)); 

    return result;  
  }

  Key* get_key_located_diagonal_top_right(GridCorner corner) {
    return get_key_located_diagonal_top_right(corner.index_row, corner.index_column);
  }

  Key* get_key_located_diagonal_top_left(GridCorner corner) {
    return get_key_located_diagonal_top_left(corner.index_row, corner.index_column);
  }

  Key* get_key_located_diagonal_bottom_left(GridCorner corner) {
    return get_key_located_diagonal_bottom_left(corner.index_row, corner.index_column);
  }

  Key* get_key_located_diagonal_bottom_right(GridCorner corner) {
    return get_key_located_diagonal_bottom_right(corner.index_row, corner.index_column);
  }

  Key* get_key_located_up(GridCorner corner) {
    return get_key_located_up(corner.index_row, corner.index_column);
  }

  Key* get_key_located_down(GridCorner corner) {
    return get_key_located_down(corner.index_row, corner.index_column);
  }

  Key* get_key_located_left(GridCorner corner) {
    return get_key_located_left(corner.index_row, corner.index_column);
  }

  Key* get_key_located_right(GridCorner corner) {
    return get_key_located_right(corner.index_row, corner.index_column);
  }

  GridCorner get_key_corner_top_left() {
    auto corners = get_key_corners();
    return corners[0];
  }

  GridCorner get_key_corner_top_right() {
    auto corners = get_key_corners();
    return corners[1];
  }

  GridCorner get_key_corner_bottom_right() {
    auto corners = get_key_corners();
    return corners[2];
  }

  GridCorner get_key_corner_bottom_left() {
    auto corners = get_key_corners();
    return corners[3];
  }

  std::vector<Key*> keys() {
    std::vector<Key*> result;
    for (auto& row : data) {
      for (Key* key : row) {
        if (key) {
          result.push_back(key);
        }
      }
    }
    return result;
  }

  size_t num_columns() {
    return data[0].size();
  }

  size_t num_rows() {
    return data.size();
  }

  std::vector<std::vector<Key*>> data;
};

// Used to connect key corners together. It is thin so it can have width issues when the two
// connectors being hulled don't have a large projection on one another. (keys close together with
// vertical separation)
Shape GetPostConnector();

Shape ConnectVertical(const Key& top,
                      const Key& bottom,
                      Shape connector = GetPostConnector(),
                      double offset = 0);
Shape ConnectHorizontal(const Key& left,
                        const Key& right,
                        Shape connector = GetPostConnector(),
                        double offset = 0);
Shape ConnectDiagonal(const Key& top_left,
                      const Key& top_right,
                      const Key& bottom_right,
                      const Key& bottom_left,
                      Shape connector = GetPostConnector(),
                      double offset = 0);

Shape Tri(const TransformList& t1,
          const TransformList& t2,
          const TransformList& t3,
          Shape connector = GetPostConnector());
Shape Tri(const Shape& s1, const Shape& s2, const Shape& s3);

Shape TriHull(const TransformList& t1,
              const TransformList& t2,
              const TransformList& t3,
              const TransformList& t4,
              Shape connector = GetPostConnector());
Shape TriHull(const Shape& s1, const Shape& s2, const Shape& s3, const Shape& s4);

Shape TriFan(const TransformList& center,
             const std::vector<TransformList>& transforms,
             Shape connector = GetPostConnector());
Shape TriFan(Shape center, const std::vector<Shape>& shapes);

// Makes a triangle with every consecutive set of 3 transforms. TriHull would be the same as
// calling this with 4 transforms.
Shape TriMesh(const std::vector<TransformList>& transforms, Shape connector = GetPostConnector());
Shape TriMesh(const std::vector<Shape>& shapes);

Shape MakeDsaCap();
Shape MakeSaCap();
Shape MakeSaEdgeCap(SaEdgeType edge_type = SaEdgeType::BOTTOM);
Shape MakeSaTallEdgeCap(SaEdgeType edge_type = SaEdgeType::BOTTOM);
Shape MakeSwitch(bool add_side_nub = true);

}  // namespace scad
