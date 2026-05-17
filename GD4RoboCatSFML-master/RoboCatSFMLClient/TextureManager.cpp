// Joseph Byrne D00255161

#include "RoboCatClientPCH.hpp"
#include <windows.h>

std::unique_ptr< TextureManager >		TextureManager::sInstance;

void TextureManager::StaticInit()
{
	sInstance.reset(new TextureManager());
}

TextureManager::TextureManager()
{
    //not currently using these
	//CacheTexture("cat", "../Assets/cat.png");
	//CacheTexture("mouse", "../Assets/mouse.png");
	//CacheTexture("yarn", "../Assets/yarn.png");#
    
    // Print working directory so we know where to put the files
    char buffer[256];
    GetCurrentDirectoryA(256, buffer);
    LOG("Working directory: %s", buffer);

    CacheTexture("player", "Assets/player.png");
    CacheTexture("player_potato", "Assets/player_potato.png");
	CacheTexture("background", "Assets/background.png");

    //normal Player
    CacheTexture("player_up", "Assets/player_up.png");
	CacheTexture("player_down", "Assets/player_down.png");
	CacheTexture("player_left", "Assets/player_left.png");
	CacheTexture("player_right", "Assets/player_right.png");
	CacheTexture("player_upleft", "Assets/player_upleft.png");
	CacheTexture("player_upright", "Assets/player_upright.png");
	CacheTexture("player_downleft", "Assets/player_downleft.png");
	CacheTexture("player_downright", "Assets/player_downright.png");

	//potato Player
	CacheTexture("player_potato_up", "Assets/player_potato_up.png");
	CacheTexture("player_potato_down", "Assets/player_potato_down.png");
	CacheTexture("player_potato_left", "Assets/player_potato_left.png");
	CacheTexture("player_potato_right", "Assets/player_potato_right.png");
	CacheTexture("player_potato_upleft", "Assets/player_potato_upleft.png");
	CacheTexture("player_potato_upright", "Assets/player_potato_upright.png");
	CacheTexture("player_potato_downleft", "Assets/player_potato_downleft.png");
	CacheTexture("player_potato_downright", "Assets/player_potato_downright.png");

	//Explosion
	CacheTexture("explosion1", "Assets/explosion1.PNG");
	CacheTexture("explosion2", "Assets/explosion2.PNG");
	CacheTexture("explosion3", "Assets/explosion3.PNG");
}

TexturePtr TextureManager::GetTexture(const string& inTextureName)
{
    auto it = mNameToTextureMap.find(inTextureName);
    if (it != mNameToTextureMap.end())
        return it->second;
	LOG("TextureManager: GetTexture failed to find texture named %s", inTextureName.c_str());
    return nullptr;
}

bool TextureManager::CacheTexture(string inTextureName, const char* inFileName)
{
	TexturePtr newTexture(new sf::Texture());
	if (!newTexture->loadFromFile(inFileName))
	{
		LOG("TextureManager: Failed to load texture from file %s", inFileName);
		return false;
	}
	mNameToTextureMap[inTextureName] = newTexture;
	return true;
}
