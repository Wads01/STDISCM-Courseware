#pragma once

#include "AGameObject.hpp"

class FPSCounter : public AGameObject {
public:
	FPSCounter();
	~FPSCounter() = default;

	void initialize() override;
	void processInput(sf::Event event) override;
	void update(sf::Time deltaTime) override;
	void draw(sf::RenderWindow* targetWindow) override;

private:
	sf::Time updateTime;
	std::optional<sf::Text> statsText;
	sf::Font font;
	int framesPassed = 0;

	void updateFPS(sf::Time elapsedTime);

};

