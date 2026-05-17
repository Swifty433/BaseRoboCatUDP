#include "RoboCatClientPCH.hpp"
#include "ExplosionEffect.hpp"

ExplosionEffect::ExplosionEffect(const Vector3& inLocation) :
    mTimer(0.f),
    mCurrentFrame(0)
{
    //put the explosion at the location passed in
    SetLocation(inLocation);

    SetScale(0.8f);
    mSprite.reset(new SpriteComponent(this));
    mSprite->SetTexture(TextureManager::sInstance->GetTexture("explosion1"));
    
}
void ExplosionEffect::Update()
{
    //Checking how long the last frame is on screen
	mTimer += Timing::sInstance.GetDeltaTime();

    //Each explosion image stays on screen for .3 seconds 3 total frames.
    const float frameTime = 0.3f;
    const int totalFrames = 3;
    const float totalAnimationTime = frameTime * totalFrames;

    //once all frames are played delete
    if (mTimer >= totalAnimationTime)
    {
        SetDoesWantToDie(true);
        return;
    }
    //finding otu which frame to be shown
    int newFrame = static_cast<int>(mTimer / frameTime);
    //change image if frame is set to be changed.
    if (newFrame != mCurrentFrame)
    {
        mCurrentFrame = newFrame;

        string textureName = StringUtils::Sprintf("explosion%d", mCurrentFrame + 1);
        mSprite->SetTexture(TextureManager::sInstance->GetTexture(textureName));
    }
}