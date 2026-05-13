// Joseph Byrne D00255161

#include "RoboCatServerPCH.hpp"
#include <fstream>
#include <algorithm>

bool Server::StaticInit()
{
    s_instance.reset(new Server());
    return true;
}

Server::Server() :
    mPotatoHolderId(-1),
    mPotatoTimer(15.f),
    mPotatoTimerMax(15.f),
    mPlayersAliveThisRound(0),
    mRoundNumber(1),
    mTotalPlayers(0),
    mRoundActive(false),
    mRoundEndTimer(0.f)
{
    // Register PotatoPlayer — replaces RoboCat/Mouse/Yarn
    GameObjectRegistry::sInstance->RegisterCreationFunction(
        'PTOP', PotatoPlayerServer::StaticCreate);

    InitNetworkManager();

    float latency = 0.f;
    string latencyString = StringUtils::GetCommandLineArg(2);
    if (!latencyString.empty())
        latency = stof(latencyString);
    NetworkManagerServer::sInstance->SetSimulatedLatency(latency);

    LoadScores();
}

int Server::Run()
{
    SetupWorld();
    return Engine::Run();
}

bool Server::InitNetworkManager()
{
    string portString = StringUtils::GetCommandLineArg(1);
    uint16_t port = stoi(portString);
    return NetworkManagerServer::StaticInit(port);
}

void Server::SetupWorld()
{
    // Empty — players spawn when clients connect
}

void Server::DoFrame()
{
    NetworkManagerServer::sInstance->ProcessIncomingPackets();
    NetworkManagerServer::sInstance->CheckForDisconnects();

    Engine::DoFrame();

    if (mRoundActive)
        UpdatePotatoTimer();
    else if (mRoundEndTimer > 0.f)
    {
        mRoundEndTimer -= Timing::sInstance.GetDeltaTime();
        if (mRoundEndTimer <= 0.f)
            StartNewRound();
    }

    NetworkManagerServer::sInstance->SendOutgoingPackets();
}

void Server::UpdatePotatoTimer()
{
    if (mPotatoHolderId < 0) return;

    mPotatoTimer -= Timing::sInstance.GetDeltaTime();

    if (mPotatoTimer <= 0.f)
    {
        PotatoPlayerPtr holder = GetPlayerForId(mPotatoHolderId);
        if (holder)
        {
            mDeathOrder.push_back(mPotatoHolderId);
            static_cast<PotatoPlayerServer*>(holder.get())->ExplodePotato();
            mPlayersAliveThisRound--;
            LOG("Player %d exploded! %d players remain",
                mPotatoHolderId, mPlayersAliveThisRound);
        }

        if (mPlayersAliveThisRound <= 1)
        {
            for (const auto& go : World::sInstance->GetGameObjects())
            {
                PotatoPlayer* pp = go->GetAsPotatoPlayer();
                if (pp && pp->IsAlive())
                {
                    mDeathOrder.push_back(pp->GetPlayerId());
                    break;
                }
            }
            EndRound();
        }
        else
        {
            PassPotatoToNearestAlivePlayer(mPotatoHolderId);
            mPotatoTimerMax = std::max(5.f, mPotatoTimerMax - 1.f);
            mPotatoTimer = mPotatoTimerMax;
        }
    }
}

void Server::PassPotatoToNearestAlivePlayer(int inFromPlayerId)
{
    PotatoPlayerPtr fromPlayer = GetPlayerForId(inFromPlayerId);
    if (!fromPlayer) return;

    Vector3 fromLoc = fromPlayer->GetLocation();
    float   closestDist = FLT_MAX;
    PotatoPlayerServerPtr closestPlayer = nullptr;

    for (const auto& go : World::sInstance->GetGameObjects())
    {
        PotatoPlayer* pp = go->GetAsPotatoPlayer();
        if (pp && pp->IsAlive() && (int)pp->GetPlayerId() != inFromPlayerId)
        {
            float dist = (pp->GetLocation() - fromLoc).LengthSq2D();
            if (dist < closestDist)
            {
                closestDist = dist;
                closestPlayer = std::static_pointer_cast<PotatoPlayerServer>(go);
            }
        }
    }

    if (closestPlayer)
    {
        fromPlayer->SetHasPotato(false);
        NetworkManagerServer::sInstance->SetStateDirty(
            fromPlayer->GetNetworkId(), PotatoPlayer::EPRS_Potato);

        closestPlayer->ReceivePotato();
        mPotatoHolderId = closestPlayer->GetPlayerId();
        LOG("Potato passed to player %d", mPotatoHolderId);
    }
}

void Server::EndRound()
{
    mRoundActive = false;

    int numPlayers = (int)mDeathOrder.size();
    for (int i = 0; i < numPlayers; ++i)
    {
        int playerId = mDeathOrder[i];
        int points = i + 1;
        mCumulativeScores[playerId] += points;
        ScoreBoardManager::sInstance->IncScore(playerId, points);
        LOG("Player %d earned %d points", playerId, points);
    }

    SaveScores();
    LOG("Round %d over! Next round in 5 seconds...", mRoundNumber);
    mRoundNumber++;
    mRoundEndTimer = 5.f;
}

void Server::StartNewRound()
{
    mDeathOrder.clear();
    mPotatoTimer = mPotatoTimerMax;
    mPotatoHolderId = -1;

    const auto& clients =
        NetworkManagerServer::sInstance->GetAddressToClientMap();

    mPlayersAliveThisRound = 0;

    float angleStep = (2.f * 3.14159f) / std::max(1, (int)clients.size());
    float angle = 0.f;
    float spawnRadius = 300.f;
    Vector3 center(640.f, 360.f, 0.f);

    for (const auto& pair : clients)
    {
        int playerId = pair.second->GetPlayerId();

        PotatoPlayerPtr old = GetPlayerForId(playerId);
        if (old) old->SetDoesWantToDie(true);

        SpawnPlayerForId(playerId);

        PotatoPlayerPtr newPlayer = GetPlayerForId(playerId);
        if (newPlayer)
        {
            Vector3 spawnPos = center + Vector3(
                cosf(angle) * spawnRadius,
                sinf(angle) * spawnRadius,
                0.f);
            newPlayer->SetLocation(spawnPos);
            angle += angleStep;
            mPlayersAliveThisRound++;
        }
    }

    if (!clients.empty())
    {
        auto it = clients.begin();
        std::advance(it, rand() % clients.size());
        int firstHolder = it->second->GetPlayerId();

        PotatoPlayerPtr holder = GetPlayerForId(firstHolder);
        if (holder)
        {
            static_cast<PotatoPlayerServer*>(holder.get())->ReceivePotato();
            mPotatoHolderId = firstHolder;
        }
    }

    mRoundActive = true;
    LOG("Round %d started! Potato with player %d, timer %.1fs",
        mRoundNumber, mPotatoHolderId, mPotatoTimer);
}

void Server::HandleNewClient(ClientProxyPtr inClientProxy)
{
    int playerId = inClientProxy->GetPlayerId();
    ScoreBoardManager::sInstance->AddEntry(playerId, inClientProxy->GetName());
    mCumulativeScores[playerId] = 0;
    mTotalPlayers++;

    SpawnPlayerForId(playerId);
    mPlayersAliveThisRound++;

    if (mTotalPlayers == 1)
    {
        PotatoPlayerPtr player = GetPlayerForId(playerId);
        if (player)
        {
            static_cast<PotatoPlayerServer*>(player.get())->ReceivePotato();
            mPotatoHolderId = playerId;
        }
        mRoundActive = true;
        LOG("First player connected, round started!", 0);
    }
}

void Server::SpawnPlayerForId(int inPlayerId)
{
    PotatoPlayerPtr player = std::static_pointer_cast<PotatoPlayer>(
        GameObjectRegistry::sInstance->CreateGameObject('PTOP'));

    player->SetColor(
        ScoreBoardManager::sInstance->GetEntry(inPlayerId)->GetColor());
    player->SetPlayerId(inPlayerId);
    player->SetLocation(Vector3(
        640.f - static_cast<float>(inPlayerId) * 60.f, 360.f, 0.f));

    LOG("Spawned PotatoPlayer for player %d", inPlayerId);
}

void Server::HandleLostClient(ClientProxyPtr inClientProxy)
{
    int playerId = inClientProxy->GetPlayerId();
    ScoreBoardManager::sInstance->RemoveEntry(playerId);
    mTotalPlayers--;

    PotatoPlayerPtr player = GetPlayerForId(playerId);
    if (player)
    {
        if ((int)player->GetPlayerId() == mPotatoHolderId)
            PassPotatoToNearestAlivePlayer(playerId);
        player->SetDoesWantToDie(true);
        mPlayersAliveThisRound--;
    }

    SaveScores();
}

PotatoPlayerPtr Server::GetPlayerForId(int inPlayerId)
{
    for (const auto& go : World::sInstance->GetGameObjects())
    {
        PotatoPlayer* pp = go->GetAsPotatoPlayer();
        if (pp && (int)pp->GetPlayerId() == inPlayerId)
            return std::static_pointer_cast<PotatoPlayer>(go);
    }
    return nullptr;
}

void Server::SaveScores()
{
    std::ofstream file("scores.txt");
    if (!file.is_open()) { LOG("Failed to save scores!", 0); return; }
    for (const auto& pair : mCumulativeScores)
        file << pair.first << " " << pair.second << "\n";
    file.close();
    LOG("Scores saved.", 0);
}

void Server::LoadScores()
{
    std::ifstream file("scores.txt");
    if (!file.is_open()) { LOG("No scores file, starting fresh.", 0); return; }
    int playerId, score;
    while (file >> playerId >> score)
        mCumulativeScores[playerId] = score;
    file.close();
    LOG("Scores loaded.", 0);
}