// Joseph Byrne D00255161

#pragma once

class PotatoPlayerServer : public PotatoPlayer
{
public:
    static GameObjectPtr StaticCreate()
    {
        return NetworkManagerServer::sInstance->RegisterAndReturn(
            new PotatoPlayerServer());
    }

    virtual void HandleDying() override;
    virtual void Update() override;

    void ReceivePotato();
    void ExplodePotato();

protected:
    PotatoPlayerServer();
};

typedef shared_ptr<PotatoPlayerServer> PotatoPlayerServerPtr;