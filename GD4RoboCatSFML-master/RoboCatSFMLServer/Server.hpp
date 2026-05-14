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
    void            SetPotatoHolder(int inPlayerId) { mPotatoHolderId = inPlayerId; }

private:
    Server();
    bool InitNetworkManager();
    void SetupWorld();

    void UpdatePotatoTimer();
    void StartNewRound();
    void SaveScores();
    void LoadScores();

    int   mPotatoHolderId;
    float mPotatoTimer;
    float mPotatoTimerMax;
    int   mTotalPlayers;

    unordered_map<int, int> mCumulativeScores;

    bool  mRoundActive;
    float mRoundEndTimer;
};