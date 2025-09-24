#include <iostream>
#include <string>

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

    if (this->ticks >= 10000.0f) {
        if (TextureManager::getInstance()->getNumLoadedStreamTextures() < TextureManager::getInstance()->streamingAssetCount) {
            TextureManager::getInstance()->loadSingleStreamAsset(TextureManager::getInstance()->getNumLoadedStreamTextures());
        }
        this->spawnObject();
        this->ticks = 0.0f;
    }
}

void TextureDisplay::spawnObject() {
    int iconIdx = TextureManager::getInstance()->getNumLoadedStreamTextures() - 1;
    if (iconIdx < 0) return;

    String objectName = "Icon_" + std::to_string(iconIdx);
    IconObject* iconObj = new IconObject(objectName, iconIdx);
    this->iconList.push_back(iconObj);

    // set position
    int IMG_WIDTH = 68, IMG_HEIGHT = 68;
    float x = this->columnGrid * IMG_WIDTH;
    float y = this->rowGrid * IMG_HEIGHT;
    iconObj->setPosition(sf::Vector2f(x, y));

    std::cout << "Set position: " << x << " " << y << std::endl;

    this->columnGrid++;
    if (this->columnGrid == this->MAX_COLUMN) {
        this->columnGrid = 0;
        this->rowGrid++;
    }
    GameObjectManager::getInstance()->addObject(iconObj);
}
