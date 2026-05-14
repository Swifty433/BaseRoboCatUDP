// Joseph Byrne D00255161

#pragma once

class PotatoPlayerClient : public PotatoPlayer
{
public:
    static GameObjectPtr StaticCreate()
    {
        return GameObjectPtr(new PotatoPlayerClient());
    }

    virtual void Update() override;
    virtual void HandleDying() override;
    virtual void Read(InputMemoryBitStream& inInputStream) override;

protected:
    PotatoPlayerClient();

private:
    void DoClientSidePredictionForLocal(uint32_t inReadState);
    void DoClientSidePredictionForRemote(uint32_t inReadState);
    void InterpolateClientSidePrediction(float inOldRotation,
        const Vector3& inOldLocation,
        const Vector3& inOldVelocity,
        bool inIsRemote);

    float mTimeLocationBecameOutOfSync;
    float mTimeVelocityBecameOutOfSync;

    PlayerSpriteComponentPtr mNormalSprite;
    PlayerSpriteComponentPtr mPotatoSprite;
};