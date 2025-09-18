#include <iostream>

#include "FPSCounter.hpp"
#include "BaseRunner.hpp"

FPSCounter::FPSCounter() : AGameObject("FPSCounter") {}

void FPSCounter::initialize() {
    if (!font.openFromFile("../Media/Sansation.ttf")) {
        std::cerr << "Failed to load font\n";
        return;
    }

    statsText.emplace(font, std::string("FPS: --\n"), 35u);
    statsText->setPosition(sf::Vector2f(BaseRunner::WINDOW_WIDTH - 175, BaseRunner::WINDOW_HEIGHT - 70));
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
    static float timeAccumulator = 0.0f;
    static int frameCount = 0;

    timeAccumulator += elapsedTime.asSeconds();
    frameCount++;

    if (timeAccumulator >= 1.0f) {
        int fps = static_cast<int>(frameCount / timeAccumulator + 0.5f);
        if (statsText) statsText->setString("FPS: " + std::to_string(fps) + "\n");
        timeAccumulator = 0.0f;
        frameCount = 0;
    }
}
