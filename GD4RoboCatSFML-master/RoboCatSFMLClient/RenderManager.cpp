//edited by joseph byrne D00255161
//eoin hamill D00258444
#include "RoboCatClientPCH.hpp"

std::unique_ptr< RenderManager >	RenderManager::sInstance;

RenderManager::RenderManager() :
	//screenshake variables
	mShakeTimer(0.f),
	mShakeDuration(0.f),
	mShakeIntensity(0.f)
{
	view.reset(sf::FloatRect(0, 0, 1920, 1080));
	WindowManager::sInstance->setView(view);

	


	if(mBackgroundTexture.loadFromFile("Assets/background.png"))
	{
		mBackgroundSprite.setTexture(mBackgroundTexture);
		float scaleX = 1920.f / mBackgroundTexture.getSize().x;
		float scaleY = 1080.f / mBackgroundTexture.getSize().y;
		mBackgroundSprite.setScale(scaleX, scaleY);
	}
	else
	{
		LOG("RenderManager: Failed to load background texture",0);
	}
}


void RenderManager::StaticInit()
{
	sInstance.reset(new RenderManager());
}


void RenderManager::AddComponent(SpriteComponent* inComponent)
{
	mComponents.emplace_back(inComponent);
}

void RenderManager::RemoveComponent(SpriteComponent* inComponent)
{
	int index = GetComponentIndex(inComponent);

	if (index != -1)
	{
		int lastIndex = mComponents.size() - 1;
		if (index != lastIndex)
		{
			mComponents[index] = mComponents[lastIndex];
		}
		mComponents.pop_back();
	}
}

int RenderManager::GetComponentIndex(SpriteComponent* inComponent) const
{
	for (int i = 0, c = mComponents.size(); i < c; ++i)
	{
		if (mComponents[i] == inComponent)
		{
			return i;
		}
	}

	return -1;
}


//this part that renders the world is really a camera-
//in a more detailed engine, we'd have a list of cameras, and then render manager would
//render the cameras in order
void RenderManager::RenderComponents()
{
	//Get the logical viewport so we can pass this to the SpriteComponents when it's draw time
	for (SpriteComponent* c : mComponents)
	{
		if (c->IsVisible())
			WindowManager::sInstance->draw(c->GetSprite());
	}
}

void RenderManager::Render()
{
	//
	// Clear the back buffer
	//
	WindowManager::sInstance->clear(sf::Color(sf::Color::White));

	UpdateScreenShake();

	WindowManager::sInstance->draw(mBackgroundSprite);

	RenderManager::sInstance->RenderComponents();

	RenderPlayerNames();

	HUD::sInstance->Render();

	view.reset(sf::FloatRect(0, 0, 1920, 1080));
	WindowManager::sInstance->setView(view);

	//
	// Present our back buffer to our front buffer
	//
	WindowManager::sInstance->display();

}
//set screenshake variables
void RenderManager::StartScreenShake(float inDuration, float inIntensity)
{
	mShakeDuration = inDuration;
	mShakeTimer = inDuration;
	mShakeIntensity = inIntensity;
}

//
void RenderManager::UpdateScreenShake()
{
	//reset camera
	view.reset(sf::FloatRect(0, 0, 1920, 1080));
	//if shake timer is above 0 screenshake
	if (mShakeTimer > 0.f)
	{
		//countdown
		mShakeTimer -= Timing::sInstance.GetDeltaTime();
		//work out how long is left
		float shakePercent = mShakeTimer / mShakeDuration;
		//shake gets weaker over time
		float currentIntensity = mShakeIntensity * shakePercent;
		//random horizonaltal and then vertical mvoement offset
		float offsetX = ((rand() % 200) / 100.f - 1.f) * currentIntensity;
		float offsetY = ((rand() % 200) / 100.f - 1.f) * currentIntensity;
		//move the camera
		view.move(offsetX, offsetY);
	}
	//apply the changes
	WindowManager::sInstance->setView(view);
}

void RenderManager::RenderPlayerNames()
{
	for (const GameObjectPtr& gameObject : World::sInstance->GetGameObjects())
	{
		PotatoPlayer* player = gameObject->GetAsPotatoPlayer();

		if (player == nullptr || !player->IsAlive())
		{
			continue;
		}

		ScoreBoardManager::Entry* entry =
			ScoreBoardManager::sInstance->GetEntry(player->GetPlayerId());

		if (entry == nullptr)
		{
			continue;
		}

		sf::Text nameText;
		nameText.setFont(*FontManager::sInstance->GetFont("carlito"));
		nameText.setString(entry->GetPlayerName());
		nameText.setCharacterSize(24);
		nameText.setFillColor(sf::Color::White);
		nameText.setOutlineColor(sf::Color::Black);
		nameText.setOutlineThickness(2.f);

		sf::FloatRect bounds = nameText.getLocalBounds();

		nameText.setOrigin(
			bounds.left + bounds.width / 2.f,
			bounds.top + bounds.height / 2.f
		);

		Vector3 playerLocation = player->GetLocation();

		nameText.setPosition(
			playerLocation.mX,
			playerLocation.mY + 85.f
		);

		WindowManager::sInstance->draw(nameText);
	}
}
