#pragma once
#include <vector>

template <typename T>
using VV = std::vector<std::vector<T>>;

template <typename T>
using V = std::vector<T>;

template <typename T>
struct Board {
  Board(int width, int height)
    : height_(height)
    , width_(width)
    , board_(height, std::vector<T>(width, T{}))
  {}
    
  Board(int size) : Board(size, size) {}
    
  T& Get(int i, int j) {
    assert(i >= 0 && i < height_);
    assert(j >= 0 && j < width_);
    return board_[i][j];
  }

  const T& Get(int i, int j) const {
    assert(i >= 0 && i < height_);
    assert(j >= 0 && j < width_);
    return board_[i][j];
  }
  
  int Height() const { return height_; }

  int Width() const { return width_; }

 private:
  const int height_;
  const int width_;
  VV<T> board_;
};