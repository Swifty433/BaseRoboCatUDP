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
}

TexturePtr TextureManager::GetTexture(const string& inTextureName)
{
    auto it = mNameToTextureMap.find(inTextureName);
    if (it != mNameToTextureMap.end())
        return it->second;
    return nullptr;
}

bool TextureManager::CacheTexture(string inTextureName, const char* inFileName)
{
	TexturePtr newTexture(new sf::Texture());
	if (!newTexture->loadFromFile(inFileName))
	{
		return false;
	}

	mNameToTextureMap[inTextureName] = newTexture;

	return true;

}
