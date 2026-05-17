//edited by joseph byrne D00255161
#include "RoboCatClientPCH.hpp"

std::unique_ptr< RenderManager >	RenderManager::sInstance;

RenderManager::RenderManager() :
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

	HUD::sInstance->Render();

	view.reset(sf::FloatRect(0, 0, 1920, 1080));
	WindowManager::sInstance->setView(view);

	//
	// Present our back buffer to our front buffer
	//
	WindowManager::sInstance->display();

}

void RenderManager::StartScreenShake(float inDuration, float inIntensity)
{
	mShakeDuration = inDuration;
	mShakeTimer = inDuration;
	mShakeIntensity = inIntensity;
}

void RenderManager::UpdateScreenShake()
{
	view.reset(sf::FloatRect(0, 0, 1920, 1080));
	
	if (mShakeTimer > 0.f)
	{
		mShakeTimer -= Timing::sInstance.GetDeltaTime();

		float shakePercent = mShakeTimer / mShakeDuration;

		float currentIntensity = mShakeIntensity * shakePercent;

		float offsetX = ((rand() % 200) / 100.f - 1.f) * currentIntensity;
		float offsetY = ((rand() % 200) / 100.f - 1.f) * currentIntensity;

		view.move(offsetX, offsetY);
	}

	WindowManager::sInstance->setView(view);
}
