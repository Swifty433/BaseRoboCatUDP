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
    mLastDeadPlayerId(-1),
    mDeathEventId(0),
    mPotatoTimer(15.f),
    mPotatoTimerMax(15.f),
    mTotalPlayers(0),
    mRoundActive(false),
    mRoundEndTimer(0.f),
    mInLobby(true),
    mLobbyStarted(false),
    mLobbyTimer(15.f)
{
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

    //Adding in lobby
    if (mInLobby)
    {
        if (mLobbyStarted)
        {
            mLobbyTimer -= Timing::sInstance.GetDeltaTime();

            // Press SPACE on the server console to skip lobby timer
            if (GetAsyncKeyState(VK_SPACE) & 0x8000)
            {
                mLobbyTimer = 0.f;
            }

            if (mLobbyTimer <= 0.f)
            {
                mLobbyTimer = 0.f;
                mInLobby = false;

                LOG("STARTING FIRST ROUND", 0);
                StartNewRound();
            }
        }
    }

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
    // Guard: don't fire if round already ended or no holder assigned
    if (mPotatoHolderId < 0 || !mRoundActive) return;

    mPotatoTimer -= Timing::sInstance.GetDeltaTime();

    if (mPotatoTimer <= 0.f)
    {
        PotatoPlayerPtr holder = GetPlayerForId(mPotatoHolderId);
        if (holder)
        {
            mLastDeadPlayerId = mPotatoHolderId;
            mDeathEventId++;

            LOG("Player %d exploded! Everyone else gets a point.", mPotatoHolderId);

            // Give all OTHER connected players 1 point
            const auto& clients =
                NetworkManagerServer::sInstance->GetAddressToClientMap();
            for (const auto& pair : clients)
            {
                int pid = pair.second->GetPlayerId();
                if (pid != mPotatoHolderId)
                {
                    mCumulativeScores[pid]++;
                    ScoreBoardManager::sInstance->IncScore(pid, 1);
                    LOG("Player %d gets 1 point", pid);
                }
            }

            SaveScores();

            // Kill the holder
            static_cast<PotatoPlayerServer*>(holder.get())->ExplodePotato();
        }

        // Everyone respawns after 3 seconds
        mRoundActive = false;
        mRoundEndTimer = 3.f;
        mPotatoTimer = GetRandomPotatoTime();
        mPotatoTimerMax = mPotatoTimer;
    }
}

void Server::StartNewRound()
{
    mPotatoTimer = GetRandomPotatoTime();
    mPotatoTimerMax = mPotatoTimer;
    mPotatoHolderId = -1;

    const auto& clients =
        NetworkManagerServer::sInstance->GetAddressToClientMap();

    if (clients.empty()) return;

    // Respawn ALL players in a circle — including whoever just exploded
    float angleStep = (2.f * 3.14159f) / std::max(1, (int)clients.size());
    float angle = 0.f;
    float spawnRadius = 300.f;
    Vector3 center(960.f, 540.f, 0.f);

    for (const auto& pair : clients)
    {
        int playerId = pair.second->GetPlayerId();

        // Get old player BEFORE spawning the new one
        PotatoPlayerPtr old = GetPlayerForId(playerId);

        // Spawn fresh first — SpawnPlayerForId now returns the new player
        PotatoPlayerPtr newPlayer = SpawnPlayerForId(playerId);

        // Kill old AFTER spawning so GetPlayerForId (which skips dying objects)
        // will return the new one from here on
        if (old) old->SetDoesWantToDie(true);

        if (newPlayer)
        {
            Vector3 spawnPos = center + Vector3(
                cosf(angle) * spawnRadius,
                sinf(angle) * spawnRadius,
                0.f);
            newPlayer->SetLocation(spawnPos);
            angle += angleStep;
        }
    }

    // Give potato to a random player
    auto it = clients.begin();
    std::advance(it, rand() % clients.size());
    int firstHolder = it->second->GetPlayerId();

    // GetPlayerForId now skips dying objects so this safely finds the new object
    PotatoPlayerPtr holder = GetPlayerForId(firstHolder);
    if (holder)
    {
        static_cast<PotatoPlayerServer*>(holder.get())->ReceivePotato();
        mPotatoHolderId = firstHolder;
    }

    mRoundActive = true;
    LOG("New round! Potato with player %d, timer %.1fs",
        mPotatoHolderId, mPotatoTimer);
}

void Server::HandleNewClient(ClientProxyPtr inClientProxy)
{
    int playerId = inClientProxy->GetPlayerId();

    ScoreBoardManager::sInstance->AddEntry(playerId, inClientProxy->GetName());
    mCumulativeScores[playerId] = 0;
    mTotalPlayers++;

    printf("Player %d (%s) joined! Total players: %d\n",
        playerId, inClientProxy->GetName().c_str(), mTotalPlayers);
    fflush(stdout);

    // First player gets the potato and starts the round
    if (mInLobby)
    {
        mLobbyStarted = true;
        SpawnPlayerForId(playerId);
        LOG("Player %d joined", playerId);
        return;
    }

    PotatoPlayerPtr player = SpawnPlayerForId(playerId);
    LOG("Player %d joined later", playerId);


}

PotatoPlayerPtr Server::SpawnPlayerForId(int inPlayerId)
{
    PotatoPlayerPtr player = std::static_pointer_cast<PotatoPlayer>(
        GameObjectRegistry::sInstance->CreateGameObject('PTOP'));

    player->SetColor(
        ScoreBoardManager::sInstance->GetEntry(inPlayerId)->GetColor());
    player->SetPlayerId(inPlayerId);
    player->SetLocation(Vector3(
        960.f - static_cast<float>(inPlayerId) * 60.f, 540.f, 0.f));

    LOG("Spawned PotatoPlayer for player %d", inPlayerId);
    return player;
}

void Server::HandleLostClient(ClientProxyPtr inClientProxy)
{
    int playerId = inClientProxy->GetPlayerId();
    ScoreBoardManager::sInstance->RemoveEntry(playerId);
    mTotalPlayers--;

    printf("Player %d disconnected. Total players: %d\n", playerId, mTotalPlayers);
    fflush(stdout);

    PotatoPlayerPtr player = GetPlayerForId(playerId);
    if (player)
    {
        // If they had the potato, give it to someone else
        if ((int)player->GetPlayerId() == mPotatoHolderId)
        {
            player->SetHasPotato(false);
            NetworkManagerServer::sInstance->SetStateDirty(
                player->GetNetworkId(), PotatoPlayer::EPRS_Potato);

            bool foundNewHolder = false;

            // Find any other living player to give it to
            for (const auto& go : World::sInstance->GetGameObjects())
            {
                PotatoPlayer* pp = go->GetAsPotatoPlayer();
                if (pp && (int)pp->GetPlayerId() != playerId && !pp->DoesWantToDie())
                {
                    PotatoPlayerServer* sp =
                        static_cast<PotatoPlayerServer*>(pp);
                    sp->ReceivePotato();
                    mPotatoHolderId = pp->GetPlayerId();
                    foundNewHolder = true;
                    break;
                }
            }

            // Nobody to give it to — force a new round after a short delay
            if (!foundNewHolder)
            {
                mRoundActive = false;
                mRoundEndTimer = 3.f;
                mPotatoHolderId = -1;
            }
        }
        player->SetDoesWantToDie(true);
    }

    SaveScores();

    if (mTotalPlayers <= 0)
    {
        mRoundActive = false;
        mPotatoHolderId = -1;

        mInLobby = true;
        mLobbyStarted = false;
        mLobbyTimer = 15.f;
    }
}

PotatoPlayerPtr Server::GetPlayerForId(int inPlayerId)
{
    for (const auto& go : World::sInstance->GetGameObjects())
    {
        PotatoPlayer* pp = go->GetAsPotatoPlayer();
        // Skip dying objects so we always return the live player
        if (pp && (int)pp->GetPlayerId() == inPlayerId && !pp->DoesWantToDie())
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

float Server::GetRandomPotatoTime()
{
    const float minTime = 5.f;
    const float maxTime = 25.f;

    float randomValue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    return minTime + randomValue * (maxTime - minTime);
}