#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>

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
	std::ifstream stream("../Media/assets.txt");
	if (!stream.is_open()) {
		std::cerr << "[TextureManager] Failed to open assets.txt" << std::endl;
		return;
	}
	
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

    if (!std::filesystem::exists(STREAMING_PATH)) {
        std::cerr << "[TextureManager] Streaming directory does not exist: " << STREAMING_PATH << std::endl;
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(STREAMING_PATH)) {
        if (index == fileNum)
        {
            // Load asset
            String path = entry.path().string();
            String filename = entry.path().filename().string();
            String assetName = StringUtils::split(filename, '.')[0];
            this->instantiateAsTexture(path, assetName, true);

            std::cout << "[TextureManager] Loaded streaming texture: " << assetName << std::endl;
            break;
        }
        fileNum++;
    }
}

sf::Texture* TextureManager::getFromTextureMap(const String assetName, int frameIndex) {
	auto it = this->textureMap.find(assetName);
	if (it != this->textureMap.end() && !it->second.empty() && frameIndex >= 0 && frameIndex < it->second.size()) {
		return it->second[frameIndex];
	}
	else {
		std::cerr << "[TextureManager] No texture found for " << assetName << " at frame " << frameIndex << std::endl;
		return nullptr;
	}
}

int TextureManager::getNumFrames(const String assetName) {
	if (!textureMap[assetName].empty()) {
		return (int)textureMap[assetName].size();
	}
	else {
		std::cout << "[TextureManager] No texture found for " << assetName << std::endl;
		return 0;
	}
}

sf::Texture* TextureManager::getStreamTextureFromList(const int index) {
	if (index < 0 || index >= streamTextureList.size()) {
		std::cerr << "[TextureManager] Stream texture index out of range: " << index << std::endl;
		return nullptr;
	}
	return streamTextureList[index];
}

int TextureManager::getNumLoadedStreamTextures() const {
	return (int)streamTextureList.size();
}

void TextureManager::countStreamingAssets() {
	streamingAssetCount = 0;
	
	// Check if directory exists before counting
	if (!std::filesystem::exists(STREAMING_PATH)) {
		std::cerr << "[TextureManager] Streaming directory does not exist: " << STREAMING_PATH << std::endl;
		return;
	}
	
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
