//Joseph Byrne D00255161

#include "RoboCatClientPCH.hpp"
#include "AudioManager.hpp"

std::unique_ptr<AudioManager> AudioManager::sInstance;

void AudioManager::StaticInt()
{
	sInstance.reset(new AudioManager());
}

AudioManager::AudioManager()
{
	PlayMusic("music.ogg");
	SetMusicVolume(50.f);
}

void AudioManager::PlayMusic(const std::string& inFilePath)
{
	if (!mMusic.openFromFile(inFilePath))
	{
		LOG("Failed to load music file %s", inFilePath.c_str());
		return;
	}
	mMusic.play();
	LOG("AudioManager: Playing music %s", inFilePath.c_str());
}

void AudioManager::StopMusic()
{
	mMusic.stop();
}

void AudioManager::SetMusicVolume(float inVolume)
{
	mMusic.setVolume(inVolume);
	LOG("AudioManager: Set music volume to %.2f", inVolume);
}