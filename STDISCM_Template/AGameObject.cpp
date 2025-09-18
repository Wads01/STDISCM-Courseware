#include "AGameObject.hpp"

AGameObject::AGameObject(String name) {
	this->name = name;
	this->sprite = nullptr;
	this->texture = nullptr;
}

AGameObject::~AGameObject() {
	if (sprite != nullptr) {
		delete sprite;
	}
	if (texture != nullptr) {
		delete texture;
	}
}

AGameObject::String AGameObject::getName() {
	return name;
}

void AGameObject::draw(sf::RenderWindow* targetWindow) {
	if (sprite != NULL) {
		sprite->setScale(scale);
		targetWindow->draw(*sprite);
	}
}

//must be called after being registered to the game object manager or one of the parent game objects
void AGameObject::setPosition(const sf::Vector2f& position) {
	this->position = position;
	if (sprite != nullptr) {
		sprite->setPosition(position);
	}
}

void AGameObject::setScale(const sf::Vector2f& scale) {
	if (sprite != nullptr) {
		sprite->setScale(scale);
	}
}

sf::Vector2f AGameObject::getPosition() {
	if (sprite != nullptr) {
		return sprite->getPosition();
	}
	return position;
}

sf::Vector2f AGameObject::getScale() {
	if (sprite != nullptr) {
		return sprite->getScale();
	}
	return scale;
}

sf::FloatRect AGameObject::getLocalBounds() {
	if (sprite != nullptr) {
		return sprite->getLocalBounds();
	}
	return sf::FloatRect();
}
