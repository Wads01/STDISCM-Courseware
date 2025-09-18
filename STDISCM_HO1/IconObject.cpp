#include <iostream>

#include "IconObject.hpp"
#include "BaseRunner.hpp"
#include "TextureManager.hpp"

IconObject::IconObject(String name, int textureIndex) : AGameObject(name) {
	textureIndex = textureIndex;
}

void IconObject::initialize() {
	//assign texture
	sf::Texture* texture = TextureManager::getInstance()->getStreamTextureFromList(textureIndex);
	sprite = new sf::Sprite(*texture);
}

void IconObject::processInput(sf::Event event) {}

void IconObject::update(sf::Time deltaTime) {}
