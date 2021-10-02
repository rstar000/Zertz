#pragma once
#include <optional>
#include <cassert>

struct QR { 
  int q, r; 
  friend bool operator== (const QR& left, const QR& right) {
    return left.q == right.q && left.r == right.r;
  }
};

enum class PileId {kPlayer1, kPlayer2, kTable};
enum class PlayerId {kPlayer1, kPlayer2};

using BallId = int;

enum class ObjType { kBall, kPile, kCell, kUndoButton };

struct ObjIndex {
  ObjType type;

  // I'm afraid to use variant here (because enum and int are very similar)
  std::optional<QR> qr;
  std::optional<BallId> ball_id;
  std::optional<PileId> pile_id;
  
  QR GetQR() {
    assert(type == ObjType::kCell && qr);
    return *qr;
  }

  BallId GetBallId() {
    assert(type == ObjType::kBall && ball_id);
    return *ball_id;
  }

  PileId GetPileId() {
    assert(type == ObjType::kPile && pile_id);
    return *pile_id;
  }
};