#pragma once

#include "AGameObject.hpp"

class BGObject : public AGameObject {
public:
	BGObject();
	~BGObject() = default;

	void initialize() override;
	void processInput(sf::Event event) override;
	void update(sf::Time deltaTime) override;

private:
	const float SPEED_MULTIPLIER = 3000.0f;
};

