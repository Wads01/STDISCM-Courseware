#include <iostream>

#include "BGObject.hpp"
#include "TextureManager.hpp"
#include "BaseRunner.hpp"

BGObject::BGObject(string name) : AGameObject(name) {}

void BGObject::initialize() {
	std::cout << "Declared as " << this->getName() << "\n";

	sf::Texture* texture = TextureManager::getInstance()->getFromTextureMap("Desert", 0);
	texture->setRepeated(true);
	sprite->setTexture(*texture);
	sf::Vector2u textureSize = sprite->getTexture().getSize();

	sprite->setTextureRect(
		sf::IntRect(
			sf::Vector2i(0, 0), 
			sf::Vector2i(BaseRunner::WINDOW_WIDTH, BaseRunner::WINDOW_HEIGHT * 8)
		)
	);
	setPosition(sf::Vector2f(0, -BaseRunner::WINDOW_HEIGHT * 7));
}

void BGObject::processInput(sf::Event event) {}

void BGObject::update(sf::Time deltaTime) {
	//make BG scroll slowly
	sf::Vector2f position = this->getPosition();
	position.y += this->SPEED_MULTIPLIER * deltaTime.asSeconds();
	this->setPosition(position);

	sf::Vector2f localPos = this->sprite->getPosition();
	if (localPos.y * deltaTime.asSeconds() > 0) {
		//reset position
		this->setPosition(sf::Vector2f(0, -BaseRunner::WINDOW_HEIGHT * 7));
	}
}
