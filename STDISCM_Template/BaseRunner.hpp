#include <SFML/Graphics.hpp>
#include <vector>

using namespace std;

class BaseRunner {
public:
	static const sf::Time TIME_PER_FRAME;
	static const int WINDOW_WIDTH = 1920;
	static const int WINDOW_HEIGHT = 1080;

	BaseRunner();

	BaseRunner(const BaseRunner&) = delete;
	BaseRunner& operator=(const BaseRunner&) = delete;

	void run();

private:

	sf::RenderWindow window;

	void render();
	void processEvents();
	void update(sf::Time elapsedTime);
};
