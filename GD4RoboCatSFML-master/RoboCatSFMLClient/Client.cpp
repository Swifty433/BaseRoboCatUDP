// Joseph Byrne D00255161
#include "RoboCatClientPCH.hpp"
#include <fstream>
#include <windows.h>

bool Client::StaticInit()
{
    // Managers must be created BEFORE Client constructor
    // so RenderManager exists when SpriteComponents are created
	InputManager::StaticInit();

	WindowManager::StaticInit();
	FontManager::StaticInit();
	TextureManager::StaticInit();
	RenderManager::StaticInit();
	

	HUD::StaticInit();
	AudioManager::StaticInt();

    Client* client = new Client();
    s_instance.reset(client);

	return true;
}

Client::Client()
{
	// Register PotatoPlayer only — replaces RoboCat/Mouse/Yarn
	GameObjectRegistry::sInstance->RegisterCreationFunction(
		'PTOP', PotatoPlayerClient::StaticCreate);

	//string destination = StringUtils::GetCommandLineArg(1);
	//string name = StringUtils::GetCommandLineArg(2);

	// Read IP and name from ip.txt next to the exe
	string destination = "127.0.0.1:50000";
	string name = "Player";

	char exePath[256];
	GetModuleFileNameA(NULL, exePath, 256);
	string exeDir = string(exePath);
	exeDir = exeDir.substr(0, exeDir.find_last_of("\\/"));
	string ipFilePath = exeDir + "\\ip.txt";

	std::ifstream ipFile(ipFilePath);
	if (ipFile.is_open())
	{
		std::getline(ipFile, destination);
		std::getline(ipFile, name);
		ipFile.close();
	}

	SocketAddressPtr serverAddress = SocketAddressFactory::CreateIPv4FromString(destination);

	NetworkManagerClient::StaticInit(*serverAddress, name);

	//NetworkManagerClient::sInstance->SetSimulatedLatency(0.0f);
}



void Client::DoFrame()
{
    InputManager::sInstance->Update();
    Engine::DoFrame();
    NetworkManagerClient::sInstance->ProcessIncomingPackets();
    RenderManager::sInstance->Render();
    NetworkManagerClient::sInstance->SendOutgoingPackets();
}

void Client::HandleEvent(sf::Event& p_event)
{
	if (p_event.type == sf::Event::KeyPressed &&
		p_event.key.code == sf::Keyboard::Escape)
	{
		WindowManager::sInstance->close();
		SetShouldKeepRunning(false);
		return;
	}

	switch (p_event.type)
	{
	case sf::Event::KeyPressed:
		InputManager::sInstance->HandleInput(EIA_Pressed, p_event.key.code);
		break;
	case sf::Event::KeyReleased:
		InputManager::sInstance->HandleInput(EIA_Released, p_event.key.code);
		break;
	default:
		break;
	}
}

bool Client::PollEvent(sf::Event& p_event)
{
	return WindowManager::sInstance->pollEvent(p_event);
}


