#pragma once
#include "AGameObject.hpp"

class IconObject : public AGameObject {
public:
	IconObject(String name, int textureIndex);
	~IconObject() = default;
	
	void initialize() override;
	void processInput(sf::Event event) override;
	void update(sf::Time deltaTime) override;

private:
	int textureIndex = 0;
};

