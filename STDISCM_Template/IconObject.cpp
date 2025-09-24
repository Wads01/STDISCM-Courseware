#include <iostream>

#include "IconObject.hpp"
#include "BaseRunner.hpp"
#include "TextureManager.hpp"

IconObject::IconObject(String name, int textureIndex) : AGameObject(name) {
	textureIndex = textureIndex;
}

void IconObject::initialize() {
    sf::Texture* texture = TextureManager::getInstance()->getStreamTextureFromList(textureIndex);
    if (!texture) {
        std::cerr << "IconObject: Failed to get stream texture for index " << textureIndex << std::endl;
        return;
    }
    sprite = new sf::Sprite(*texture);

    sprite->setPosition(this->position);
}

void IconObject::processInput(sf::Event event) {}

void IconObject::update(sf::Time deltaTime) {}
