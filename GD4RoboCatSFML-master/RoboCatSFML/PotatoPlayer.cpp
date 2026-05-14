// Joseph Byrne D00255161
// Eoin Hamill D00258444

#include "RoboCatPCH.hpp"

const float PP_WORLD_HEIGHT = 720.f;
const float PP_WORLD_WIDTH = 1280.f;

PotatoPlayer::PotatoPlayer() :
    GameObject(),
    mVelocity(Vector3::Zero),
    mThrustDir(0.f),
    mHealth(1),
    mHasPotato(false),
    mPlayerId(0),
    mMaxLinearSpeed(3000.f),
    mMaxRotationSpeed(180.f),
    mWallRestitution(0.1f),
    mPlayerRestitution(0.1f),
    mLastMoveTimestamp(0.f)
{
    SetCollisionRadius(40.f);
    SetScale(0.5f);
}

void PotatoPlayer::ProcessInput(float inDeltaTime, const InputState& inInputState)
{
    float newRotation = GetRotation() +
        inInputState.GetDesiredHorizontalDelta() * mMaxRotationSpeed * inDeltaTime;
    SetRotation(newRotation);
    mThrustDir = inInputState.GetDesiredVerticalDelta();
}

void PotatoPlayer::AdjustVelocityByThrust(float inDeltaTime)
{
    Vector3 forwardVector = GetForwardVector();
    mVelocity = forwardVector * (mThrustDir * inDeltaTime * mMaxLinearSpeed);
}

void PotatoPlayer::SimulateMovement(float inDeltaTime)
{
    AdjustVelocityByThrust(inDeltaTime);
    SetLocation(GetLocation() + mVelocity * inDeltaTime);
    ProcessCollisions();
}

void PotatoPlayer::Update()
{
    // subclasses handle update
}

void PotatoPlayer::ProcessCollisions()
{
    ProcessCollisionsWithScreenWalls();

    float   sourceRadius = GetCollisionRadius();
    Vector3 sourceLocation = GetLocation();

    for (auto goIt = World::sInstance->GetGameObjects().begin();
        goIt != World::sInstance->GetGameObjects().end(); ++goIt)
    {
        GameObject* target = goIt->get();
        if (target != this && !target->DoesWantToDie())
        {
            Vector3 targetLocation = target->GetLocation();
            float   targetRadius = target->GetCollisionRadius();
            Vector3 delta = targetLocation - sourceLocation;
            float   distSq = delta.LengthSq2D();
            float   collisionDist = sourceRadius + targetRadius;

            if (distSq < collisionDist * collisionDist)
            {
                Vector3 dirToTarget = delta;
                dirToTarget.Normalize2D();
                SetLocation(targetLocation - dirToTarget * collisionDist);

                Vector3 relVel = mVelocity;

                PotatoPlayer* targetPlayer = target->GetAsPotatoPlayer();
                if (targetPlayer)
                    relVel -= targetPlayer->mVelocity;

                float relVelDotDir = Dot2D(relVel, dirToTarget);
                if (relVelDotDir > 0.f)
                {
                    Vector3 impulse = relVelDotDir * dirToTarget;
                    if (targetPlayer)
                    {
                        mVelocity -= impulse;
                        mVelocity *= mPlayerRestitution;
                    }
                    else
                    {
                        mVelocity -= impulse * 2.f;
                        mVelocity *= mWallRestitution;
                    }
                }
            }
        }
    }
}

void PotatoPlayer::ProcessCollisionsWithScreenWalls()
{
    Vector3 location = GetLocation();
    float x = location.mX, y = location.mY;
    float vx = mVelocity.mX, vy = mVelocity.mY;
    float radius = GetCollisionRadius();

    if ((y + radius) >= PP_WORLD_HEIGHT && vy > 0)
    {
        mVelocity.mY = -vy * mWallRestitution;
        location.mY = PP_WORLD_HEIGHT - radius;
        SetLocation(location);
    }
    else if (y - radius <= 0 && vy < 0)
    {
        mVelocity.mY = -vy * mWallRestitution;
        location.mY = radius;
        SetLocation(location);
    }

    if ((x + radius) >= PP_WORLD_WIDTH && vx > 0)
    {
        mVelocity.mX = -vx * mWallRestitution;
        location.mX = PP_WORLD_WIDTH - radius;
        SetLocation(location);
    }
    else if (x - radius <= 0 && vx < 0)
    {
        mVelocity.mX = -vx * mWallRestitution;
        location.mX = radius;
        SetLocation(location);
    }
}

uint32_t PotatoPlayer::Write(OutputMemoryBitStream& inOutputStream,
    uint32_t inDirtyState) const
{
    uint32_t writtenState = 0;

    // PlayerId
    if (inDirtyState & EPRS_PlayerId)
    {
        inOutputStream.Write(true);
        inOutputStream.Write(mPlayerId);
        writtenState |= EPRS_PlayerId;
    }
    else { inOutputStream.Write(false); }

    // Pose — velX, velY, posX, posY, rotation
    if (inDirtyState & EPRS_Pose)
    {
        inOutputStream.Write(true);
        inOutputStream.Write(mVelocity.mX);
        inOutputStream.Write(mVelocity.mY);
        Vector3 loc = GetLocation();
        inOutputStream.Write(loc.mX);
        inOutputStream.Write(loc.mY);
        inOutputStream.Write(GetRotation());
        writtenState |= EPRS_Pose;
    }
    else { inOutputStream.Write(false); }

    // ThrustDir — 2 bits total
    if (mThrustDir != 0.f)
    {
        inOutputStream.Write(true);
        inOutputStream.Write(mThrustDir > 0.f);
    }
    else { inOutputStream.Write(false); }

    // Color
    if (inDirtyState & EPRS_Color)
    {
        inOutputStream.Write(true);
        inOutputStream.Write(GetColor());
        writtenState |= EPRS_Color;
    }
    else { inOutputStream.Write(false); }

    // Health — 1 bit (alive or dead)
    if (inDirtyState & EPRS_Health)
    {
        inOutputStream.Write(true);
        inOutputStream.Write(mHealth > 0);
        writtenState |= EPRS_Health;
    }
    else { inOutputStream.Write(false); }

    // HasPotato — 1 bit
    if (inDirtyState & EPRS_Potato)
    {
        inOutputStream.Write(true);
        inOutputStream.Write(mHasPotato);
        writtenState |= EPRS_Potato;
    }
    else { inOutputStream.Write(false); }

    return writtenState;
}