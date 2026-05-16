// Joseph Byrne D00255161
// Eoin Hamill D00258444

#pragma once

class Server : public Engine
{
public:
    static bool StaticInit();

    virtual void DoFrame() override;
    virtual int  Run();

    void HandleNewClient(ClientProxyPtr inClientProxy);
    void HandleLostClient(ClientProxyPtr inClientProxy);

    PotatoPlayerPtr GetPlayerForId(int inPlayerId);
    PotatoPlayerPtr SpawnPlayerForId(int inPlayerId);

    void SetPotatoHolder(int inPlayerId) { mPotatoHolderId = inPlayerId; }

    float GetRandomPotatoTime();

    bool IsInLobby() const { return mInLobby; }
    float GetLobbyTimeRemaining() const { return mLobbyTimer; }

private:
    Server();
    bool InitNetworkManager();
    void SetupWorld();

    void UpdatePotatoTimer();
    void StartNewRound();
    void SaveScores();
    void LoadScores();





    unordered_map<int, int> mCumulativeScores;

    int mPotatoHolderId;
    float PotatoTimer;
    float mPotatoTimerMax;
    int mTotalPlayers;
    float mPotatoTimer;
    

    bool  mRoundActive;
    float mRoundEndTimer;

    bool  mInLobby;
    bool  mLobbyStarted;
    float mLobbyTimer;

};