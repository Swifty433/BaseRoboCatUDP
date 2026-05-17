//I take care of rendering things!

class HUD
{
public:

	static void StaticInit();
	static std::unique_ptr< HUD >	sInstance;

	void Render();

	//void SetPlayerHealth(int inHealth) { mHealth = inHealth; }
	void SetDashCooldown(float inRemaining, float inMax);

	void ShowDeathMessage(uint32_t inPlayerId);

private:

	HUD();

	void	RenderBandWidth();
	void	RenderRoundTripTime();
	void	RenderScoreBoard();
	void	RenderDashCooldown();
	void	RenderText(const string& inStr, const Vector3& origin, const Vector3& inColor);
	void RenderLobby();
	void RenderDeathMessage();

	string mDeathMessage;
	float mDeathMessageTimer;

	Vector3										mBandwidthOrigin;
	Vector3										mRoundTripTimeOrigin;
	Vector3										mScoreBoardOrigin;
	Vector3										mScoreOffset;
	Vector3										mDashOffset;
	float										mDashCooldownRemaining;
	float										mDashCooldownMax;
};



