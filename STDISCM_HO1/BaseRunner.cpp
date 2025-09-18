#include "BaseRunner.hpp"
#include "GameObjectManager.hpp"
#include "BGObject.hpp"
#include "TextureManager.hpp"
#include "TextureDisplay.hpp"
#include "FPSCounter.hpp"

/// <summary>
/// This demonstrates a running parallax background where after X seconds, a batch of assets will be streamed and loaded.
/// </summary>
const sf::Time BaseRunner::TIME_PER_FRAME = sf::seconds(1.f / 60.f);

BaseRunner::BaseRunner() :
	window(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), "Template Program", sf::Style::Close) {
	//load initial textures
	TextureManager::getInstance()->loadFromAssetList();

	//load objects
	BGObject* bgObject = new BGObject();
	GameObjectManager::getInstance()->addObject(bgObject);

	TextureDisplay* display = new TextureDisplay();
	GameObjectManager::getInstance()->addObject(display);

	FPSCounter* fpsCounter = new FPSCounter();
	GameObjectManager::getInstance()->addObject(fpsCounter);
}

void BaseRunner::run() {
	sf::Clock clock;
	while (this->window.isOpen())
	{
		processEvents();

		sf::Time elapsedTime = clock.restart();
		update(elapsedTime);

		render();
	}
}

void BaseRunner::processEvents()
{
	while (const std::optional<sf::Event> event = this->window.pollEvent())
	{
		// Window closed
		if (event->is<sf::Event::Closed>()) {
			this->window.close();
			continue;
		}

		// Pass all events to the game object manager
		GameObjectManager::getInstance()->processInput(*event);
	}
}

void BaseRunner::update(sf::Time elapsedTime) {
	GameObjectManager::getInstance()->update(elapsedTime);
}

void BaseRunner::render() {
	this->window.clear();
	GameObjectManager::getInstance()->draw(&this->window);
	this->window.display();
}