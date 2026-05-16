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

    CacheTexture("player", "player.png");
    CacheTexture("player_potato", "player_potato.png");

    //normal Player
    CacheTexture("player_up", "player_up.png");
	CacheTexture("player_down", "player_down.png");
	CacheTexture("player_left", "player_left.png");
	CacheTexture("player_right", "player_right.png");
	CacheTexture("player_upleft", "player_upleft.png");
	CacheTexture("player_upright", "player_upright.png");
	CacheTexture("player_downleft", "player_downleft.png");
	CacheTexture("player_downright", "player_downright.png");

	//potato Player
	CacheTexture("player_potato_up", "player_potato_up.png");
	CacheTexture("player_potato_down", "player_potato_down.png");
	CacheTexture("player_potato_left", "player_potato_left.png");
	CacheTexture("player_potato_right", "player_potato_right.png");
	CacheTexture("player_potato_upleft", "player_potato_upleft.png");
	CacheTexture("player_potato_upright", "player_potato_upright.png");
	CacheTexture("player_potato_downleft", "player_potato_downleft.png");
	CacheTexture("player_potato_downright", "player_potato_downright.png");
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
