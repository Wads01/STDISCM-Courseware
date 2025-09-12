#include <stddef.h>

#include "GameObjectManager.hpp"
#include <iostream>

GameObjectManager* GameObjectManager::sharedInstance = NULL;

GameObjectManager* GameObjectManager::getInstance() {
	if (sharedInstance == NULL) {
		sharedInstance = new GameObjectManager();
	}

	return sharedInstance;
}

AGameObject* GameObjectManager::findObjectByName(AGameObject::String name) {
	if (gameObjectMap[name] != NULL) {
		return gameObjectMap[name];
	}
	else {
		std::cout << "Object " << name << " not found!";
		return NULL;
	}
}

List GameObjectManager::getAllObjects() {
	return gameObjectList;
}

int GameObjectManager::activeObjects() {
	return gameObjectList.size();
}

void GameObjectManager::processInput(sf::Event event) {
	for (int i = 0; i < gameObjectList.size(); i++) {
		gameObjectList[i]->processInput(event);
	}
}

void GameObjectManager::update(sf::Time deltaTime) {
	//std::cout << "Delta time: " << deltaTime.asSeconds() << "\n";
	for (int i = 0; i < gameObjectList.size(); i++) {
		gameObjectList[i]->update(deltaTime);
	}
}

//draws the object if it contains a sprite
void GameObjectManager::draw(sf::RenderWindow* window) {
	for (int i = 0; i < gameObjectList.size(); i++) {
		gameObjectList[i]->draw(window);
	}
}

void GameObjectManager::addObject(AGameObject* gameObject) {
	//also initialize the oject
	gameObjectMap[gameObject->getName()] = gameObject;
	gameObjectList.push_back(gameObject);
	gameObjectMap[gameObject->getName()]->initialize();
}

//also frees up allocation of the object.
void GameObjectManager::deleteObject(AGameObject* gameObject) {
	gameObjectMap.erase(gameObject->getName());

	int index = -1;
	for (int i = 0; i < gameObjectList.size(); i++) {
		if (gameObjectList[i] == gameObject) {
			index = i;
			break;
		}
	}

	if (index != -1) {
		gameObjectList.erase(gameObjectList.begin() + index);
	}

	delete gameObject;
}

void GameObjectManager::deleteObjectByName(AGameObject::String name) {
	AGameObject* object = findObjectByName(name);

	if (object != NULL) {
		deleteObject(object);
	}
}
