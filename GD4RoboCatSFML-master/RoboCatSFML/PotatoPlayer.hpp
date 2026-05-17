// Joseph Byrne D00255161

#pragma once

class PotatoPlayer : public GameObject
{
public:
    CLASS_IDENTIFICATION('PTOP', PotatoPlayer)

        enum EPotatoReplicationState
    {
        EPRS_Pose = 1 << 0,
        EPRS_Color = 1 << 1,
        EPRS_PlayerId = 1 << 2,
        EPRS_Health = 1 << 3,
        EPRS_Potato = 1 << 4,

        EPRS_AllState = EPRS_Pose | EPRS_Color | EPRS_PlayerId | EPRS_Health | EPRS_Potato
    };

    static GameObject* StaticCreate() { return new PotatoPlayer(); }

    virtual uint32_t      GetAllStateMask()    const override { return EPRS_AllState; }
    virtual PotatoPlayer* GetAsPotatoPlayer()        override { return this; }

    virtual void Update() override;

    void ProcessInput(float inDeltaTime, const InputState& inInputState);
    void SimulateMovement(float inDeltaTime);
    void ProcessCollisions();
    void ProcessCollisionsWithScreenWalls();

    void           SetPlayerId(uint32_t inPlayerId) { mPlayerId = inPlayerId; }
    uint32_t       GetPlayerId()              const { return mPlayerId; }

    void           SetVelocity(const Vector3& inVelocity) { mVelocity = inVelocity; }
    const Vector3& GetVelocity()              const { return mVelocity; }

    bool HasPotato()               const { return mHasPotato; }
    void SetHasPotato(bool inHas) { mHasPotato = inHas; }

    int  GetHealth()               const { return mHealth; }
    void SetHealth(int inHealth) { mHealth = inHealth; }
    bool IsAlive()                 const { return mHealth > 0; }

    virtual uint32_t Write(OutputMemoryBitStream& inOutputStream,
        uint32_t inDirtyState) const override;

protected:
    PotatoPlayer();
    void AdjustVelocityByThrust(float inDeltaTime);

    Vector3  mVelocity;
    Vector3 mMoveInput;
    float    mThrustDir;
    int      mHealth;
    bool     mHasPotato;
    uint32_t mPlayerId;

    float mMaxLinearSpeed;
    float mMaxRotationSpeed;
    float mWallRestitution;
    float mPlayerRestitution;
    float mLastMoveTimestamp;
};

typedef shared_ptr<PotatoPlayer> PotatoPlayerPtr;