#include <iostream>

#include "TextureDisplay.hpp"
#include "TextureManager.hpp"
#include "BaseRunner.hpp"
#include "GameObjectManager.hpp"
#include "IconObject.hpp"

TextureDisplay::TextureDisplay() : AGameObject("TextureDisplay") {}

void TextureDisplay::initialize() {}

void TextureDisplay::processInput(sf::Event event) {}

void TextureDisplay::update(sf::Time deltaTime) {
	this->ticks += BaseRunner::TIME_PER_FRAME.asMilliseconds();

	//TODO: <code here for spawning icon object periodically>
}

void TextureDisplay::spawnObject() {
	String objectName = "Icon_" + to_string(this->iconList.size());
	IconObject* iconObj = new IconObject(objectName, this->iconList.size());
	this->iconList.push_back(iconObj);

	//set position
	int IMG_WIDTH = 68; int IMG_HEIGHT = 68;
	float x = this->columnGrid * IMG_WIDTH;
	float y = this->rowGrid * IMG_HEIGHT;
	iconObj->setPosition(sf::Vector2f(x, y));

	std::cout << "Set position: " << x << " " << y << std::endl;

	this->columnGrid++;

	if (this->columnGrid == this->MAX_COLUMN){
		this->columnGrid = 0;
		this->rowGrid++;
	}
	GameObjectManager::getInstance()->addObject(iconObj);
}
