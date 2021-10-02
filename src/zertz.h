#pragma once

#include <iostream>
#include <map>
#include <cmath>
#include <array>
#include <vector>
#include <algorithm>
#include <cassert>
#include <optional>
#include <variant>
#include <memory>

#include "game.h"
#include "structures.h"

struct Ball {
  using Position = std::variant<PileId, QR>;
  using Id = int;

  enum class Color { kWhite, kGrey, kBlack };
  Ball(Color c, Position pos) : color(c), position(pos) {}

  const Color color;
  Position position;
  
  bool OnBoard() const {
    return std::holds_alternative<QR>(position);
  }
  
  PileId GetPile() const {
    assert(!OnBoard());
    return std::get<PileId>(position);
  }

  QR GetQR() const {
    assert(OnBoard());
    return std::get<QR>(position);
  }
};

class Pile {
 public:
  using Id = PileId;

  void Add(int ball_id) {
    auto it = ball_id_to_index.find(ball_id);
    assert(it == ball_id_to_index.end());
    ball_ids.push_back(ball_id);
    RebuildIndex();
  }
  
  void Remove(int ball_id) {
    auto it = ball_id_to_index.find(ball_id);
    assert(it != ball_id_to_index.end());
    ball_ids.erase(ball_ids.begin() + it->second);
    RebuildIndex();
  }
  
  std::optional<int> GetIndex(int ball_id) const {
    auto it = ball_id_to_index.find(ball_id);
    if (it != ball_id_to_index.end()) {
      return it->second;
    }
    return std::nullopt;
  }
  
 private:
  void RebuildIndex() {
    ball_id_to_index.clear();
    for (int i = 0; i < ball_ids.size(); ++i) {
      ball_id_to_index[ball_ids[i]] = i;
    }
  }
  std::vector<int> ball_ids;
  std::map<int, int> ball_id_to_index;
};

struct Cell {
  bool present = false;
  std::optional<int> ball_idx;
};

struct ZBoard : public Board<Cell> {
  // Board with flat side, size 2N + 1;
  ZBoard(int N) : Board(2 * N + 1) {
    for (int q = 0; q < Height(); ++q) {
      for (int r = 0; r < Width(); ++r) {
        // Make cells outside of main hexagon absent.
        if ((r < N && q < N - r) || (r > N && q > 3 * N - r)) {
          Hex(q, r).present = false;
        } else {
          Hex(q, r).present = true;
        }
      }
    }
  }
  
  Cell& Hex(QR pos) { return Hex(pos.q, pos.r); }
  const Cell& Hex(QR pos) const { return Hex(pos.q, pos.r); }
  Cell& Hex(int q, int r) { return Get(q, r); }
  const Cell& Hex(int q, int r) const { return Get(q, r); }
};

class Zertz {
 public:
  struct GameState {
    ZBoard board;
    std::vector<Ball> balls;
    std::map<PileId, Pile> piles;
  };

  Zertz() : history_({InitState()}) { }
  
  GameState Latest() const { return history_.back(); }
  
  // The possible moves. 
  // Returns true if move was successful and state was updated.
  // Otherwise, returns false.
  bool MoveToBoard(int ball_idx, QR to) const;
  bool MoveToPile(int ball_idx, PileId to) const;
  bool RemoveCell(QR pos) const;
  bool Undo() const;
  
 private:
  void Evolve(GameState new_state) const {
    history_.push_back(std::move(new_state));
  }
  
  GameState InitState() const;
 
  mutable std::vector<GameState> history_;
};

using ZState = Zertz::GameState;