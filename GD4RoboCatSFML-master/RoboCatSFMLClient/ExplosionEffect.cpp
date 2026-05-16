#include "RoboCatClientPCH.hpp"
#include "ExplosionEffect.hpp"

ExplosionEffect::ExplosionEffect(const Vector3& inLocation) :
	mTimer(0.f),
	mCurrentFrame(0)
{
	SetLocation(inLocation);
	SetScale(5.f);

	SetTexture(TextureManager::sInstance->GetTexture("explosion_1"));
}

void ExplosionEffect::Update()
{
	mTimer += Timing::sInstance.GetDeltaTime();

	int newFrame = static_cast<int>(mTimer / 0.75f);

	if (newFrame != mCurrentFrame)
	{
		mCurrentFrame = newFrame;

		if (mCurrentFrame == 0)
		{
			SetTexture(TextureManager::sInstance->GetTexture("explosion_1"));
		}
		else if (mCurrentFrame == 1)
		{
			SetTexture(TextureManager::sInstance->GetTexture("explosion_2"));
		}
		else if (mCurrentFrame == 2)
		{
			SetTexture(TextureManager::sInstance->GetTexture("explosion_2"));
		}
		else
		{
			SetDoesWantToDie(true);
		}
	}
}