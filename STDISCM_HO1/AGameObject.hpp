#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class AGameObject {
public:
	typedef std::string String;

	AGameObject(String name);
	~AGameObject();

	AGameObject(const AGameObject&) = delete;
	AGameObject& operator=(const AGameObject&) = delete;

	virtual void initialize() = 0;
	virtual void processInput(sf::Event event) = 0;
	virtual void update(sf::Time deltaTime) = 0;
	virtual void draw(sf::RenderWindow* targetWindow);
	String getName();

	virtual void setPosition(const sf::Vector2f& position);
	virtual void setScale(const sf::Vector2f& scale);
	virtual sf::FloatRect getLocalBounds();
	virtual sf::Vector2f getPosition();
	virtual sf::Vector2f getScale();

protected:
	String name;
	sf::Sprite* sprite;
	sf::Texture* texture;

	sf::Vector2f position{ 0.0f, 0.0f };
	sf::Vector2f scale{ 1.0f, 1.0f };
};

