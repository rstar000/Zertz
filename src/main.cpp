#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cassert>
#include <optional>
#include <memory>
#include <functional>

#include <SFML/Graphics.hpp>

#include "zertz.h"
#include "controller.h"
#include "gui.h"

void UpdateLMB(IOContext& io) {
  bool pressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
  if (io.left_btn_down && !pressed) {
    io.left_btn_down = false;
    io.left_btn_up = true;
    return;
  } 
  
  if (io.left_btn_up) {
    io.left_btn_up = false;
    return;
  }
  
  if (pressed) {
    io.left_btn_down = true;
    io.left_btn_up = false;
  }
}

void UpdateIO(sf::RenderWindow& win, IOContext& io) {
  auto pos = sf::Mouse::getPosition(win);
  io.mouse_coords = sf::Vector2f(pos.x, pos.y);
  UpdateLMB(io);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(1920, 1440), "The Game");

    // Model, view, controller
    auto zertz = std::make_shared<Zertz>();
    auto controller = std::make_shared<ZController>(zertz);
    auto gui = std::make_shared<ZGui>(zertz, controller);

    IOContext io;
    sf::Clock clock;
    while (window.isOpen()) {
      sf::Event event;
      while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
      }
      window.clear();
      UpdateIO(window, io);
      auto time = clock.getElapsedTime();
      gui->Draw(window, time.asSeconds(), zertz->Latest(), io);
      window.display();
    }

    return 0;
}
