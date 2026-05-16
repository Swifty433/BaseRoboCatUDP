//edited by joseph byrne D00255161
#include "RoboCatClientPCH.hpp"

std::unique_ptr< RenderManager >	RenderManager::sInstance;

RenderManager::RenderManager()
{
	view.reset(sf::FloatRect(0, 0, 1920, 1080));
	WindowManager::sInstance->setView(view);

	if(mBackgroundTexture.loadFromFile("background.png"))
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

	WindowManager::sInstance->draw(mBackgroundSprite);

	RenderManager::sInstance->RenderComponents();

	HUD::sInstance->Render();

	//
	// Present our back buffer to our front buffer
	//
	WindowManager::sInstance->display();

}
