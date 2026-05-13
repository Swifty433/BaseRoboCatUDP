// Joseph Byrne D00255161

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
    void            SpawnPlayerForId(int inPlayerId);

private:
    Server();
    bool InitNetworkManager();
    void SetupWorld();

    void UpdatePotatoTimer();
    void PassPotatoToNearestAlivePlayer(int inFromPlayerId);
    void EndRound();
    void StartNewRound();
    void SaveScores();
    void LoadScores();

    int   mPotatoHolderId;
    float mPotatoTimer;
    float mPotatoTimerMax;
    int   mPlayersAliveThisRound;
    int   mRoundNumber;
    int   mTotalPlayers;

    unordered_map<int, int> mCumulativeScores;
    vector<int>             mDeathOrder;

    bool  mRoundActive;
    float mRoundEndTimer;
};