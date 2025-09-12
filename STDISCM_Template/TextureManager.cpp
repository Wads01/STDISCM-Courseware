#include <iostream>
#include <fstream>
#include <filesystem>

#include "TextureManager.hpp"
#include "StringUtils.hpp"
#include "IETThread.hpp"

// Singleton class
TextureManager* TextureManager::sharedInstance = NULL;

TextureManager* TextureManager::getInstance() {
	if (sharedInstance == NULL) {
		sharedInstance = new TextureManager();
	}

	return sharedInstance;
}

TextureManager::TextureManager() {
	countStreamingAssets();
}

void TextureManager::loadFromAssetList() {
	std::cout << "[TextureManager] Reading from asset list" << std::endl;
	std::ifstream stream("Media/assets.txt");
	String path;

	while (std::getline(stream, path)) {
		std::vector<String> tokens = StringUtils::split(path, '/');
		String assetName = StringUtils::split(tokens[tokens.size() - 1], '.')[0];
		this->instantiateAsTexture(path, assetName, false);
		std::cout << "[TextureManager] Loaded texture: " << assetName << std::endl;
	}
}

void TextureManager::loadSingleStreamAsset(int index) {
	int fileNum = 0;

	for (const auto& entry : std::filesystem::directory_iterator(STREAMING_PATH)) {
		if (index == fileNum)
		{
			//TODO: <code here for thread sleeping. Fill this up only when instructor told so.>


			//TODO: <code here for loading asset>
			String assetName = "";

			std::cout << "[TextureManager] Loaded streaming texture: " << assetName << std::endl;

			break;
		}

		fileNum++;
	}
}

sf::Texture* TextureManager::getFromTextureMap(const String assetName, int frameIndex) {
	if (!this->textureMap[assetName].empty()) {
		return this->textureMap[assetName][frameIndex];
	}
	else {
		std::cout << "[TextureManager] No texture found for " << assetName << std::endl;
		return NULL;
	}
}

int TextureManager::getNumFrames(const String assetName) {
	if (!textureMap[assetName].empty()) {
		return textureMap[assetName].size();
	}
	else {
		std::cout << "[TextureManager] No texture found for " << assetName << std::endl;
		return 0;
	}
}

sf::Texture* TextureManager::getStreamTextureFromList(const int index) {
	return streamTextureList[index];
}

int TextureManager::getNumLoadedStreamTextures() const {
	return streamTextureList.size();
}

void TextureManager::countStreamingAssets() {
	streamingAssetCount = 0;
	for (const auto& entry : std::filesystem::directory_iterator(STREAMING_PATH)) {
		streamingAssetCount++;
	}
	std::cout << "[TextureManager] Number of streaming assets: " << streamingAssetCount << std::endl;
}

void TextureManager::instantiateAsTexture(String path, String assetName, bool isStreaming) {
	sf::Texture* texture = new sf::Texture();
	if (!texture->loadFromFile(path)) {
		std::cerr << "[TextureManager] Failed to load texture: " << path << std::endl;
		delete texture;

		return;
	}
	textureMap[assetName].push_back(texture);

	if (isStreaming) {
		streamTextureList.push_back(texture);
	}
	else {
		baseTextureList.push_back(texture);
	}

}
