#include <iostream>

#include "FPSCounter.hpp"
#include "BaseRunner.hpp"

FPSCounter::FPSCounter() : AGameObject("FPSCounter") {}

void FPSCounter::initialize() {
    if (!font.openFromFile("Media/Sansation.ttf")) {
        std::cerr << "Failed to load font\n";
    }

    statsText.emplace(std::string("FPS: --\n"), font, 35);
    statsText->setPosition(sf::Vector2f(BaseRunner::WINDOW_WIDTH - 150, BaseRunner::WINDOW_HEIGHT - 70));
    statsText->setOutlineColor(sf::Color(255, 255, 255));
    statsText->setOutlineThickness(2.5f);
}

void FPSCounter::processInput(sf::Event event) {}

void FPSCounter::update(sf::Time deltaTime) {
    updateFPS(deltaTime);
}

void FPSCounter::draw(sf::RenderWindow* targetWindow) {
    AGameObject::draw(targetWindow);
    if (statsText) targetWindow->draw(*statsText);
}

void FPSCounter::updateFPS(sf::Time elapsedTime) {
    if (statsText) statsText->setString("FPS: --\n");
}
