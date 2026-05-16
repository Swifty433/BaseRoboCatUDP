//Joseph Byrne D00255161

#pragma once
#include <SFML/Audio.hpp>

class AudioManager
{
	public: 
		static void StaticInt();
		static std::unique_ptr<AudioManager> sInstance;

		void PlayMusic(const std::string& inFilePath);
		void StopMusic();
		void SetMusicVolume(float inVolume);

		void PlaySound(const std::string& inFilePath);

	private:
		AudioManager();

		sf::Music mMusic;

		//sound buffer 
		sf::SoundBuffer mSoundBuffer;
		sf::Sound       mSound;
};

