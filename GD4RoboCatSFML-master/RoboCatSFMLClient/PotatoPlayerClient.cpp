// Joseph Byrne D00255161

#include "RoboCatClientPCH.hpp"
#include "ExplosionEffect.hpp"

//PotatoPlayerClient::PotatoPlayerClient() :
//    mTimeLocationBecameOutOfSync(0.f),
//    mTimeVelocityBecameOutOfSync(0.f)
//{
//    TexturePtr normalTex = TextureManager::sInstance->GetTexture("player");
//    TexturePtr potatoTex = TextureManager::sInstance->GetTexture("player_potato");
//
//    LOG("PotatoPlayerClient constructor - normalTex: %s, potatoTex: %s",
//        normalTex ? "OK" : "NULL",
//        potatoTex ? "OK" : "NULL");
//
//    if (normalTex && potatoTex)
//    {
//        mNormalSprite.reset(new PlayerSpriteComponent(this));
//        mNormalSprite->SetTexture(normalTex);
//        mNormalSprite->SetVisibility(true);
//
//        mPotatoSprite.reset(new PlayerSpriteComponent(this));
//        mPotatoSprite->SetTexture(potatoTex);
//        mPotatoSprite->SetVisibility(false);
//    }
//    else
//    {
//        LOG("WARNING: textures not loaded, sprites will be null!", 0);
//    }
//}

PotatoPlayerClient::PotatoPlayerClient() :
    mTimeLocationBecameOutOfSync(0.f),
    mTimeVelocityBecameOutOfSync(0.f)
{
    // Single sprite component — PlayerSpriteComponent handles direction and potato sprite swapping internally
    mNormalSprite.reset(new PlayerSpriteComponent(this));
    mNormalSprite->SetVisibility(true);
    mPotatoSprite = nullptr;
}

void PotatoPlayerClient::HandleDying()
{
    GameObject::HandleDying();
}

void PotatoPlayerClient::Update()
{
    //// Always swap sprite every frame based on current mHasPotato value
    //if (mNormalSprite && mPotatoSprite)
    //{
    //    mNormalSprite->SetVisibility(!mHasPotato);
    //    mPotatoSprite->SetVisibility(mHasPotato);
    //}

    if (GetPlayerId() == (uint32_t)NetworkManagerClient::sInstance->GetPlayerId())
    {
        const Move* pendingMove = InputManager::sInstance->GetAndClearPendingMove();
        if (pendingMove)
        {
            ProcessInput(pendingMove->GetDeltaTime(), pendingMove->GetInputState());
            SimulateMovement(pendingMove->GetDeltaTime());
        }
    }
    else
    {
        SimulateMovement(Timing::sInstance.GetDeltaTime());
        if (RoboMath::Is2DVectorEqual(GetVelocity(), Vector3::Zero))
            mTimeLocationBecameOutOfSync = 0.f;
    }
}

void PotatoPlayerClient::Read(InputMemoryBitStream& inInputStream)
{
    bool     stateBit;
    uint32_t readState = 0;

    // PlayerId
    inInputStream.Read(stateBit);
    if (stateBit)
    {
        uint32_t playerId;
        inInputStream.Read(playerId);
        SetPlayerId(playerId);
        readState |= EPRS_PlayerId;
    }

    float   oldRotation = GetRotation();
    Vector3 oldLocation = GetLocation();
    Vector3 oldVelocity = GetVelocity();

    // Pose
    inInputStream.Read(stateBit);
    if (stateBit)
    {
        Vector3 vel, loc;
        float   rot;
        inInputStream.Read(vel.mX);
        inInputStream.Read(vel.mY);
        inInputStream.Read(loc.mX);
        inInputStream.Read(loc.mY);
        inInputStream.Read(rot);

        inInputStream.Read(mDashCooldownRemaining);
        inInputStream.Read(mDashTimeRemaining);

        SetVelocity(vel);
        SetLocation(loc);
        SetRotation(rot);
        readState |= EPRS_Pose;
    }

    // ThrustDir
    inInputStream.Read(stateBit);
    if (stateBit)
    {
        bool isPositive;
        inInputStream.Read(isPositive);
        mThrustDir = isPositive ? 1.f : -1.f;
    }
    else { mThrustDir = 0.f; }

    // Color
    inInputStream.Read(stateBit);
    if (stateBit)
    {
        Vector3 color;
        inInputStream.Read(color);
        SetColor(color);
        readState |= EPRS_Color;
    }

    // Health
    inInputStream.Read(stateBit);
    if (stateBit)
    {
        bool isAlive;
        inInputStream.Read(isAlive);
        mHealth = isAlive ? 1 : 0;
        readState |= EPRS_Health;
        if (GetPlayerId() == (uint32_t)NetworkManagerClient::sInstance->GetPlayerId())
            HUD::sInstance->SetPlayerHealth(mHealth);
    }

    // HasPotato — always read the value when dirty bit is set
    inInputStream.Read(stateBit);
    if (stateBit)
    {
        bool hasPotato;
        inInputStream.Read(hasPotato);

		//play the pass sound if we just got the potato
		if (hasPotato && !mHasPotato && GetPlayerId() == (uint32_t)NetworkManagerClient::sInstance->GetPlayerId())
        {
            AudioManager::sInstance->PlaySound("Assets/pass.wav");
        }

        mHasPotato = hasPotato;
        readState |= EPRS_Potato;

        LOG("Player %d hasPotato: %s",
            GetPlayerId(), mHasPotato ? "YES" : "NO");
    }

    if (GetPlayerId() == (uint32_t)NetworkManagerClient::sInstance->GetPlayerId())
    {
        DoClientSidePredictionForLocal(readState);
        if ((readState & EPRS_PlayerId) == 0)
            InterpolateClientSidePrediction(oldRotation, oldLocation, oldVelocity, false);
    }
    else
    {
        DoClientSidePredictionForRemote(readState);
        if ((readState & EPRS_PlayerId) == 0)
            InterpolateClientSidePrediction(oldRotation, oldLocation, oldVelocity, true);
    }
}

void PotatoPlayerClient::DoClientSidePredictionForLocal(uint32_t inReadState)
{
    if ((inReadState & EPRS_Pose) != 0)
    {
        const MoveList& moveList = InputManager::sInstance->GetMoveList();
        for (const Move& move : moveList)
        {
            ProcessInput(move.GetDeltaTime(), move.GetInputState());
            SimulateMovement(move.GetDeltaTime());
        }
    }
}

void PotatoPlayerClient::DoClientSidePredictionForRemote(uint32_t inReadState)
{
    if ((inReadState & EPRS_Pose) != 0)
    {
        float rtt = NetworkManagerClient::sInstance->GetRoundTripTime();
        float deltaTime = 1.f / 30.f;
        while (true)
        {
            if (rtt < deltaTime) { SimulateMovement(rtt); break; }
            SimulateMovement(deltaTime);
            rtt -= deltaTime;
        }
    }
}

void PotatoPlayerClient::InterpolateClientSidePrediction(
    float inOldRotation,
    const Vector3& inOldLocation,
    const Vector3& inOldVelocity,
    bool inIsRemote)
{
    (void)inOldRotation;
    float rtt = NetworkManagerClient::sInstance->GetRoundTripTime();
    float time = Timing::sInstance.GetFrameStartTime();

    if (!RoboMath::Is2DVectorEqual(inOldLocation, GetLocation()))
    {
        if (mTimeLocationBecameOutOfSync == 0.f)
            mTimeLocationBecameOutOfSync = time;

        float duration = time - mTimeLocationBecameOutOfSync;
        if (duration < rtt)
            SetLocation(Lerp(inOldLocation, GetLocation(),
                inIsRemote ? (duration / rtt) : 0.1f));
    }
    else { mTimeLocationBecameOutOfSync = 0.f; }

    if (!RoboMath::Is2DVectorEqual(inOldVelocity, GetVelocity()))
    {
        if (mTimeVelocityBecameOutOfSync == 0.f)
            mTimeVelocityBecameOutOfSync = time;

        float duration = time - mTimeVelocityBecameOutOfSync;
        if (duration < rtt)
            SetVelocity(Lerp(inOldVelocity, GetVelocity(),
                inIsRemote ? (duration / rtt) : 0.1f));
    }
    else { mTimeVelocityBecameOutOfSync = 0.f; }
}