#include <iostream>

#include "BGObject.hpp"
#include "TextureManager.hpp"
#include "BaseRunner.hpp"

BGObject::BGObject() : AGameObject("BGObject") {}

void BGObject::initialize() {
	std::cout << "Declared as " << this->getName() << "\n";

	sf::Texture* texture = TextureManager::getInstance()->getFromTextureMap("Desert", 0);
	if (!texture) {
		std::cerr << "Error: Failed to load texture 'Desert' - BGObject will not render properly!" << std::endl;
		return;
	}

	// Set texture properties
	texture->setRepeated(true);

	// Create the sprite with the texture
	if (sprite != nullptr) {
		delete sprite; 
	}
	
	try {
		sprite = new sf::Sprite(*texture);
		std::cout << "BGObject sprite created successfully" << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Error creating sprite: " << e.what() << std::endl;
		sprite = nullptr;
		return;
	}

	// Set texture rectangle
	sprite->setTextureRect(
		sf::IntRect(
			sf::Vector2i(0, 0), 
			sf::Vector2i(BaseRunner::WINDOW_WIDTH, BaseRunner::WINDOW_HEIGHT * 8)
		)
	);
	
	// Set initial position
	setPosition(sf::Vector2f(0, -BaseRunner::WINDOW_HEIGHT * 7));
}

void BGObject::processInput(sf::Event event) {}

void BGObject::update(sf::Time deltaTime) {
	if (!sprite) return;

	// Scroll background
	sf::Vector2f position = this->getPosition();
	position.y += this->SPEED_MULTIPLIER * deltaTime.asSeconds();
	this->setPosition(position);

	// Reset when the background has fully scrolled down
	if (position.y >= 0) {
		this->setPosition(sf::Vector2f(0, -BaseRunner::WINDOW_HEIGHT * 7));
	}
}
