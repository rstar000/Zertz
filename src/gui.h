#pragma once

#include <cmath>
#include <memory>

#include <SFML/Graphics.hpp>

#include "structures.h"
#include "zertz.h"
#include "controller.h"

struct IOContext {
  sf::Vector2f mouse_coords;
  bool left_btn_down = false;
  bool right_btn_down = false;
  bool left_btn_up = false;
  bool right_btn_up = false;
};

struct Drawable {
  using Ptr = std::shared_ptr<Drawable>;

  Drawable(std::unique_ptr<sf::Shape> shape, float hit_radius)
    : shape(std::move(shape)), hit_radius(hit_radius) { }
  
  virtual bool IsHovered(const sf::Vector2f& mouse_ndc) const;
  void SetOutline(float time);
  
  virtual void Draw(sf::RenderWindow& win, float time, const ZState& state) = 0;
  virtual bool IsVisible(const ZState& state) const { return true; }
  virtual ObjIndex GetIndex() const = 0;
  
  std::unique_ptr<sf::Shape> shape;
  sf::Vector2f center;
  float hit_radius;
  
  bool is_hovered = false;
  bool is_selected = false;
};

struct HexDrawable : public Drawable {
  using Ptr = std::shared_ptr<HexDrawable>;

  HexDrawable(QR pos);

  void Draw(sf::RenderWindow& win, float time, const ZState& state) override;
  
  bool IsVisible(const ZState& state) const override {
    auto& cell = state.board.Hex(pos);
    return cell.present;
  }
  
  ObjIndex GetIndex() const override {
    return ObjIndex{.type = ObjType::kCell, .qr = pos};
  }

 private:
  const float radius;
  const QR pos;
};

struct BallDrawable : public Drawable {
  using Ptr = std::shared_ptr<BallDrawable>;
  BallDrawable(int idx);
  
  void Draw(sf::RenderWindow& win, float time, const ZState& state) override;

  bool IsVisible(const ZState& state) const override {
    return true;
  }
  
  ObjIndex GetIndex() const override {
    return ObjIndex{.type = ObjType::kBall, .ball_id = idx };
  }

 private:
  void SetColor(const ZState& state);
  void SetPositon(const ZState& state);
  
  const int idx;
  float radius;
};

struct PileDrawable : public Drawable {
  using Ptr = std::shared_ptr<PileDrawable>;
  PileDrawable(PileId idx);

  void Draw(sf::RenderWindow& win, float time, const ZState& state) override;

  ObjIndex GetIndex() const override {
    return ObjIndex{.type = ObjType::kPile, .pile_id = idx};
  }

 private:
  void SetColor();
  PileId idx;
};

struct UndoButtonDrawable : public Drawable {
  UndoButtonDrawable();

  void Draw(sf::RenderWindow& win, float time, const ZState& state) override;
  
  bool IsHovered(const sf::Vector2f& mouse_pos) const override;

  ObjIndex GetIndex() const override {
    return ObjIndex{.type = ObjType::kUndoButton};
  }
};

struct ZGui {
  ZGui(std::shared_ptr<Zertz> zertz, std::shared_ptr<ZController> controller)
    : objects(InitObjects(zertz->Latest()))
    , controller(controller) {}

  std::vector<std::shared_ptr<Drawable>> objects;
  std::shared_ptr<ZController> controller;
  
  std::vector<std::shared_ptr<Drawable>> InitObjects(const ZState& state) const {
    std::vector<std::shared_ptr<Drawable>> objects;
    sf::Vector2f base(100.0f, 100.0f);
    for (int h = 0; h < state.board.Height(); ++h) {
      for (int w = 0; w < state.board.Width(); ++w) {
        QR qr{h, w};
        objects.push_back(std::make_shared<HexDrawable>(qr));
      }
    }
    
    for (const auto [pile_id, _] : state.piles) {
      objects.push_back(std::make_shared<PileDrawable>(pile_id));
    }

    for (int ball_id = 0; ball_id < state.balls.size(); ++ball_id) {
      objects.push_back(std::make_shared<BallDrawable>(ball_id));
    }
    
    objects.push_back(std::make_shared<UndoButtonDrawable>());
    
    return objects;
  }
  
  void Draw(sf::RenderWindow& win, float time, const ZState& state, const IOContext& io) {
    std::optional<int> hovered_idx;
    for (int i = 0; i < objects.size(); ++i) {
      auto& obj = objects[i];
      obj->is_hovered = false;
      if (obj->IsVisible(state) && obj->IsHovered(io.mouse_coords)) {
        hovered_idx = i;
      }
    }

    if (hovered_idx) {
      auto& obj = objects[*hovered_idx];
      obj->is_hovered = true;
    }
    
    Drawable::Ptr hovered, selected;
    for (auto& obj : objects) {
      if (obj->is_hovered) {
        hovered = obj;
      } else if (obj->is_selected) {
        selected = obj;
      } else {
        obj->Draw(win, time, state);
      }
    }
    
    if (hovered) {
      hovered->Draw(win, time, state);
    }

    if (selected) {
      selected->Draw(win, time, state);
    }

    if (hovered_idx) {
      auto& obj = objects[*hovered_idx];
      obj->is_hovered = true;

      if (io.left_btn_up) {
        auto res = controller->OnClick(obj->GetIndex());
        switch (res) {
          case ActionResult::kSuccess:
          case ActionResult::kFail:
            ClearStates();
            break;
          case ActionResult::kSelected:
            obj->is_selected = true;
            break;
        }
      }
    }
  }
  
  void ClearStates() {
    for (auto& obj : objects) {
      obj->is_hovered = false;
      obj->is_selected = false;
    }
  }
};