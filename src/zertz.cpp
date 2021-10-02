#include "zertz.h"

bool Zertz::MoveToBoard(int ball_idx, QR to) const {
  GameState state = Latest();

  auto& ball = state.balls[ball_idx];
  auto& hex = state.board.Hex(to);
  if (!hex.present || hex.ball_idx) {
    return false;
  }
  
  // If it was on board, remove it from old hex
  if (ball.OnBoard()) {
    auto& old_hex = state.board.Hex(std::get<QR>(ball.position));
    old_hex.ball_idx = {};
  } else {
    auto& old_pile = state.piles[ball.GetPile()];
    old_pile.Remove(ball_idx);
  }

  hex.ball_idx = ball_idx;
  ball.position = to;
  Evolve(state);
  return true;
}

bool Zertz::MoveToPile(int ball_idx, PileId to) const {
  GameState state = Latest();
  auto& ball = state.balls[ball_idx];
  if (!ball.OnBoard() && ball.GetPile() == to) {
    return false;
  }
  
  if (ball.OnBoard()) {
    auto& old_hex = state.board.Hex(std::get<QR>(ball.position));
    old_hex.ball_idx = {};
  } else {
    auto& old_pile = state.piles[ball.GetPile()];
    old_pile.Remove(ball_idx);
  }
  
  ball.position = to;
  auto& new_pile = state.piles.at(to);
  new_pile.Add(ball_idx);

  auto index_in_pile = new_pile.GetIndex(ball_idx);
  assert(index_in_pile);
  Evolve(state);
  return true;
}

bool Zertz::RemoveCell(QR pos) const {
  GameState state = Latest();
  auto& hex = state.board.Hex(pos);
  if (!hex.present || hex.ball_idx) {
    return false;
  }
  
  hex.present = false;
  Evolve(state);
  return true;
}

bool Zertz::Undo() const {
  if (history_.size() == 1) {
    return false;
  }
  history_.pop_back();
  return true;
}


Zertz::GameState Zertz::InitState() const {
  GameState state{.board = ZBoard(3), .balls = {}, .piles = {}};
  state.piles[PileId::kTable] = Pile{};
  state.piles[PileId::kPlayer1] = Pile{};
  state.piles[PileId::kPlayer2] = Pile{};
  int ball_id = 0;
  auto AddBalls = [&] (int n, auto color) {
    for (int i = 0; i < n; ++i) {
      state.balls.emplace_back(color, PileId::kTable);
      state.piles[PileId::kTable].Add(ball_id);
      ++ball_id;
    }
  };

  AddBalls(6, Ball::Color::kWhite);
  AddBalls(8, Ball::Color::kGrey);
  AddBalls(10, Ball::Color::kBlack);
  return state;
}