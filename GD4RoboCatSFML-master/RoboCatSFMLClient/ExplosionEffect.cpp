#include "RoboCatClientPCH.hpp"
#include "ExplosionEffect.hpp"

ExplosionEffect::ExplosionEffect(const Vector3& inLocation) :
    mTimer(0.f),
    mCurrentFrame(0)
{
    SetLocation(inLocation);

    SetScale(0.4f);
    mSprite.reset(new SpriteComponent(this));
    mSprite->SetTexture(TextureManager::sInstance->GetTexture("explosion1"));
    
}
void ExplosionEffect::Update()
{
	mTimer += Timing::sInstance.GetDeltaTime();

    const float frameTime = 0.3f;
    const int totalFrames = 3;
    const float totalAnimationTime = frameTime * totalFrames;

    if (mTimer >= totalAnimationTime)
    {
        SetDoesWantToDie(true);
        return;
    }

    int newFrame = static_cast<int>(mTimer / frameTime);

    if (newFrame != mCurrentFrame)
    {
        mCurrentFrame = newFrame;

        string textureName = StringUtils::Sprintf("explosion%d", mCurrentFrame + 1);
        mSprite->SetTexture(TextureManager::sInstance->GetTexture(textureName));
    }
}