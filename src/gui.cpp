#include "gui.h"

namespace {

const float kMargin = 100.0f;
const float kBallRadius = 30.0f;
const float kCellSize = 40.0f;
const sf::Vector2f kBoardBase = sf::Vector2f(100.0f, 100.0f);
const sf::Vector2f kPilesBase = sf::Vector2f(100.0f, 600.0f);
const sf::Vector2f kUndoButtonPos = sf::Vector2f(700.0f, 100.0f);
const sf::Vector2f kUndoButtonSize = sf::Vector2f(200.0f, 100.0f);


sf::Vector2f QRCenterOffset(QR pos, float radius) {
  float sq3 = std::sqrt(3.0f);
  float sq32 = sq3 / 2.0f;
  float x = radius * (3.0f/2 * pos.q);
  float y = radius * (sq32 * pos.q + sq3 * pos.r);
  return sf::Vector2f(x, y);
}

std::vector<sf::Vector2f> MakeHexVertices(float rad) {
  float sq3 = sqrtf(3.0f);
  float sq32 = sq3 / 2.0f;
  sf::Vector2f a{rad, 0};
  sf::Vector2f b{rad * 0.5f, rad * sq32};
  sf::Vector2f c{-rad * 0.5f, rad * sq32};
  sf::Vector2f d{-rad, 0};
  sf::Vector2f e{-rad * 0.5f, -rad * sq32};
  sf::Vector2f f{rad * 0.5f, -rad * sq32};

  return {a, b, c, d, e, f};
}

float GetPileYOffset(PileId pile_id) {
  switch (pile_id) {
  case PileId::kPlayer1:
    return 100.0f;
  case PileId::kPlayer2:
    return 200.0f;
  case PileId::kTable:
    return 300.0f;
  }
}

float GetBallXOffset(int index_in_pile) {
  return (1 + index_in_pile) * 2.2f * kBallRadius;
}

}

bool Drawable::IsHovered(const sf::Vector2f& mouse_ndc) const {
  float dx = mouse_ndc.x - center.x;
  float dy = mouse_ndc.y - center.y;
  return dx * dx + dy * dy < hit_radius * hit_radius;
}

sf::Color Interp(sf::Color a, sf::Color b, float mix) {
  return sf::Color(
    a.r * mix + b.r * (1.0f - mix),
    a.g * mix + b.g * (1.0f - mix),
    a.r * mix + b.b * (1.0f - mix));
}

void Drawable::SetOutline(float time) {
  if (is_selected) {
    shape->setScale(sf::Vector2f(1.2f, 1.2f));
    shape->setOutlineThickness(5.0f);
    shape->setOutlineColor(sf::Color::Red);
  } else if (is_hovered) {
    shape->setOutlineThickness(5.0f);
    shape->setOutlineColor(sf::Color::Yellow);
  } else {
    shape->setScale(sf::Vector2f(1.0f, 1.0f));
    shape->setOutlineThickness(0.0f);
  }
}

std::unique_ptr<sf::Shape> MakeShape(float radius) {
  auto v = MakeHexVertices(radius);
  auto shape = std::make_unique<sf::ConvexShape>(6);
  for (int i = 0; i < v.size(); ++i) {
    shape->setPoint(i, v[i]);
  }

  return shape; 
}

HexDrawable::HexDrawable(QR pos) 
  : Drawable(MakeShape(kCellSize), kCellSize)
  , radius(kCellSize)
  , pos(pos) {
  center = kBoardBase + QRCenterOffset(pos, kCellSize);
  shape->setPosition(center);
}


void HexDrawable::Draw(sf::RenderWindow& win, float time, const ZState& state) {
  if (!IsVisible(state)) {
    return;
  }
  
  SetOutline(time);

  if (std::abs(100 + pos.r - pos.q) % 3 == 0) {
    shape->setFillColor(sf::Color(50, 250, 150));
  } else if (std::abs(100 + pos.r - pos.q) % 3 == 1) {
    shape->setFillColor(sf::Color(70, 200, 150));
  } else {
    shape->setFillColor(sf::Color(60, 220, 170));
  }
  
  shape->setPosition(center);
  win.draw(*shape);
}


BallDrawable::BallDrawable(int idx) 
  : Drawable(std::make_unique<sf::CircleShape>(kBallRadius)
  , kBallRadius)
  , idx(idx)
  , radius(kBallRadius) {
    shape->setOrigin(sf::Vector2f(radius, radius));
  }
  
void BallDrawable::Draw(sf::RenderWindow& win, float time, const ZState& state) {
  SetPositon(state);
  SetColor(state);
  SetOutline(time);
  win.draw(*shape);
}

void BallDrawable::SetColor(const ZState& state) {
  auto& ball = state.balls[idx];
  switch (ball.color) {
    case Ball::Color::kBlack:
      shape->setFillColor(sf::Color(50, 50, 50));
      break;
    case Ball::Color::kWhite:
      shape->setFillColor(sf::Color(255, 255, 255));
      break;
    case Ball::Color::kGrey:
      shape->setFillColor(sf::Color(150, 150, 150));
      break;
  }
}

void BallDrawable::SetPositon(const ZState& state) {
  auto& ball = state.balls[idx];
  if (ball.OnBoard()) {
    center = kBoardBase + QRCenterOffset(ball.GetQR(), kCellSize);
  } else {
    auto pile_id = ball.GetPile();
    const auto& pile = state.piles.at(pile_id);
    auto index_in_pile = pile.GetIndex(idx);
    assert(index_in_pile);
    center = kPilesBase + sf::Vector2f(
        GetBallXOffset(*index_in_pile), 
        GetPileYOffset(pile_id));
  }
  shape->setPosition(center);
}

PileDrawable::PileDrawable(PileId idx) 
  : Drawable(std::make_unique<sf::RectangleShape>(
        sf::Vector2f(kBallRadius * 2.0f, kBallRadius * 2.0f)),
        kBallRadius)
  , idx(idx)  {
  shape->setOrigin(kBallRadius, kBallRadius);
}

void PileDrawable::Draw(sf::RenderWindow& win, float time, const ZState& state) {
  center = kPilesBase + sf::Vector2f(0.0f, GetPileYOffset(idx));
  shape->setPosition(center);
  SetOutline(time);
  SetColor();
  win.draw(*shape);
}

void PileDrawable::SetColor() {
  switch (idx) {
    case PileId::kPlayer1:
      shape->setFillColor(sf::Color(100, 220, 100));
      break;
    case PileId::kPlayer2:
      shape->setFillColor(sf::Color(50, 100, 200));
      break;
    case PileId::kTable:
      shape->setFillColor(sf::Color(200, 100, 100));
      break;
  }
}


UndoButtonDrawable::UndoButtonDrawable() 
  : Drawable(std::make_unique<sf::RectangleShape>(kUndoButtonSize), 10.0f)
  {}
  
bool UndoButtonDrawable::IsHovered(const sf::Vector2f& mouse_pos) const {
  return mouse_pos.x > kUndoButtonPos.x && mouse_pos.y > kUndoButtonPos.y &&
      mouse_pos.x < kUndoButtonPos.x + kUndoButtonSize.x && 
      mouse_pos.y < kUndoButtonPos.y + kUndoButtonSize.y;
}

void UndoButtonDrawable::Draw(sf::RenderWindow& win, float time, const ZState& state) {
  SetOutline(time);
  shape->setFillColor(sf::Color::Magenta);
  shape->setPosition(kUndoButtonPos);
  win.draw(*shape);
}
