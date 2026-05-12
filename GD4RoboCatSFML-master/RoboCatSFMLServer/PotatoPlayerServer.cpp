// Joseph Byrne D00255161

#include "RoboCatServerPCH.hpp"

PotatoPlayerServer::PotatoPlayerServer() {}

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