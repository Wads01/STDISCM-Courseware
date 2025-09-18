#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "AGameObject.hpp"
#include <SFML/Graphics.hpp>

/* 
Singleton Class
Game object manager contains all of the declared game object classes and calls the update function
 */

typedef std::unordered_map<std::string, AGameObject*> HashTable;
typedef std::vector<AGameObject*> List;

class GameObjectManager {
public:
	static GameObjectManager* getInstance();

	void processInput(sf::Event event);
	void update(sf::Time deltaTime);
	void draw(sf::RenderWindow* window);

	AGameObject* findObjectByName(AGameObject::String name);
	List getAllObjects();

	int activeObjects();
	void addObject(AGameObject* gameObject);
	void deleteObject(AGameObject* gameObject);
	void deleteObjectByName(AGameObject::String name);

private:
	GameObjectManager() {};
	GameObjectManager(GameObjectManager const&) {};             // copy constructor is private
	GameObjectManager& operator=(GameObjectManager const&) {};  // assignment operator is private
	static GameObjectManager* sharedInstance;

	HashTable gameObjectMap;
	List gameObjectList;
};

