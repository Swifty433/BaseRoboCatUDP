// Joseph Byrne D00255161

#include "RoboCatServerPCH.hpp"

PotatoPlayerServer::PotatoPlayerServer() :
    mPassCooldown(0.f)
{}

void PotatoPlayerServer::HandleDying()
{
    NetworkManagerServer::sInstance->UnregisterGameObject(this);
}

void PotatoPlayerServer::Update()
{
    
    Vector3 oldLocation = GetLocation();
    Vector3 oldVelocity = GetVelocity();
    float   oldRotation = GetRotation();
    bool    oldHasPotato = HasPotato();

    ClientProxyPtr client =
        NetworkManagerServer::sInstance->GetClientProxy(GetPlayerId());
    if (client)
    {
        MoveList& moveList = client->GetUnprocessedMoveList();
        for (const Move& move : moveList)
        {
            ProcessInput(move.GetDeltaTime(), move.GetInputState());
            SimulateMovement(move.GetDeltaTime());
        }
        moveList.Clear();
    }

    // If this player has the potato, check for collision to pass it
    if (mPassCooldown > 0.f)
    {
        mPassCooldown -= Timing::sInstance.GetDeltaTime();

        if (mPassCooldown < 0.f)
        {
            mPassCooldown = 0.f;
        }
    }

    if (HasPotato() && mPassCooldown <= 0.f)
    {
        float   sourceRadius = GetCollisionRadius();
        Vector3 sourceLocation = GetLocation();

        for (const auto& go : World::sInstance->GetGameObjects())
        {
            PotatoPlayer* target = go->GetAsPotatoPlayer();
            if (target && target != this && !target->DoesWantToDie())
            {
                Vector3 delta = target->GetLocation() - sourceLocation;
                float   distSq = delta.LengthSq2D();
                float   collisionDist = sourceRadius + target->GetCollisionRadius();

                if (distSq < collisionDist * collisionDist)
                {
                    // Pass potato to this player
                    SetHasPotato(false);
                    NetworkManagerServer::sInstance->SetStateDirty(
                        GetNetworkId(), PotatoPlayer::EPRS_Potato);

                    PotatoPlayerServer* targetServer =
                        static_cast<PotatoPlayerServer*>(target);
                    targetServer->ReceivePotato();
                    mPassCooldown = 0.75f;

                    // Tell the server who holds it now
                    static_cast<Server*>(Engine::s_instance.get())
                        ->SetPotatoHolder(target->GetPlayerId());

                    LOG("Player %d passed potato to player %d!",
                        GetPlayerId(), target->GetPlayerId());
                    break;
                }
            }
        }
    }

    if (!RoboMath::Is2DVectorEqual(oldLocation, GetLocation()) ||
        !RoboMath::Is2DVectorEqual(oldVelocity, GetVelocity()) ||
        oldRotation != GetRotation())
    {
        NetworkManagerServer::sInstance->SetStateDirty(
            GetNetworkId(), PotatoPlayer::EPRS_Pose);
    }

    if (oldHasPotato != HasPotato())
    {
        NetworkManagerServer::sInstance->SetStateDirty(
            GetNetworkId(), PotatoPlayer::EPRS_Potato);
    }
}

void PotatoPlayerServer::ReceivePotato()
{
    SetHasPotato(true);
    mPassCooldown = 0.75f;

    NetworkManagerServer::sInstance->SetStateDirty(
        GetNetworkId(), PotatoPlayer::EPRS_Potato);
}

void PotatoPlayerServer::ExplodePotato()
{
    SetHasPotato(false);
    mHealth = 0;
    SetDoesWantToDie(true);
    NetworkManagerServer::sInstance->SetStateDirty(
        GetNetworkId(), PotatoPlayer::EPRS_Health);
    NetworkManagerServer::sInstance->SetStateDirty(
        GetNetworkId(), PotatoPlayer::EPRS_Potato);
}