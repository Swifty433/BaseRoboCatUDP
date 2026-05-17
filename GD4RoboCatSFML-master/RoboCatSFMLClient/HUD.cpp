#include "RoboCatClientPCH.hpp"

std::unique_ptr< HUD >	HUD::sInstance;


HUD::HUD() :
	mScoreBoardOrigin(50.f, 60.f, 0.0f),
	mBandwidthOrigin(50.f, 10.f, 0.0f),
	mRoundTripTimeOrigin(580.f, 10.f, 0.0f),
	mScoreOffset(0.f, 50.f, 0.0f),
	mDashOffset(1000.f, 10.f,0.0f),
	mDashCooldownRemaining(0.f),
	mDashCooldownMax(5.f),
	mDeathMessage(""),
	mDeathMessageTimer(0.f)
{
}


void HUD::StaticInit()
{
	sInstance.reset(new HUD());
}

void HUD::Render()
{
	RenderBandWidth();
	RenderRoundTripTime();
	RenderScoreBoard();

	if (NetworkManagerClient::sInstance->IsInLobby())
	{
		RenderLobby();
		return;
	}
	else {
		RenderDashCooldown();
		RenderDeathMessage();
	}
}

void HUD::RenderDashCooldown()
{
	if (mDashCooldownRemaining <= 0.f)
	{
		RenderText("Dash Ready", mDashOffset, Colors::Green);
	}
	else
	{
		string dashString = StringUtils::Sprintf("Dash %.1fs", mDashCooldownRemaining);
		RenderText(dashString, mDashOffset, Colors::Red);
	}
}

void HUD::SetDashCooldown(float inRemaining, float inMax)
{
	mDashCooldownRemaining = inRemaining;
	mDashCooldownMax = inMax;

	if (mDashCooldownRemaining < 0.f)
	{
		mDashCooldownRemaining = 0.f;
	}
}

void HUD::RenderBandWidth()
{
	string bandwidth = StringUtils::Sprintf("In %d  Bps, Out %d Bps",
		static_cast<int>(NetworkManagerClient::sInstance->GetBytesReceivedPerSecond().GetValue()),
		static_cast<int>(NetworkManagerClient::sInstance->GetBytesSentPerSecond().GetValue()));
	RenderText(bandwidth, mBandwidthOrigin, Colors::White);
}

void HUD::RenderRoundTripTime()
{
	float rttMS = NetworkManagerClient::sInstance->GetAvgRoundTripTime().GetValue() * 1000.f;

	string roundTripTime = StringUtils::Sprintf("RTT %d ms", (int)rttMS);
	RenderText(roundTripTime, mRoundTripTimeOrigin, Colors::White);
}

void HUD::RenderScoreBoard()
{
	const vector< ScoreBoardManager::Entry >& entries = ScoreBoardManager::sInstance->GetEntries();
	Vector3 offset = mScoreBoardOrigin;

	for (const auto& entry : entries)
	{
		RenderText(entry.GetFormattedNameScore(), offset, entry.GetColor());
		offset.mX += mScoreOffset.mX;
		offset.mY += mScoreOffset.mY;
	}

}

void HUD::RenderText(const string& inStr, const Vector3& origin, const Vector3& inColor)
{
	sf::Text text;
	text.setString(inStr);
	text.setFillColor(sf::Color(inColor.mX, inColor.mY, inColor.mZ, 255));
	text.setCharacterSize(28);
	text.setPosition(origin.mX, origin.mY);
	text.setFont(*FontManager::sInstance->GetFont("carlito"));
	WindowManager::sInstance->draw(text);
}

void HUD::RenderLobby()
{
	float timeRemaining =
		NetworkManagerClient::sInstance->GetLobbyTimeRemaining();

	int seconds = static_cast<int>(timeRemaining);
	if (seconds < 0)
		seconds = 0;

	string title = "LOBBY";
	string message = StringUtils::Sprintf("Game starts in %d seconds", seconds);
	string waiting = "Waiting for players...";

	RenderText(title, Vector3(850.f, 360.f, 0.f), Colors::White);
	RenderText(message, Vector3(760.f, 430.f, 0.f), Colors::White);
	RenderText(waiting, Vector3(780.f, 500.f, 0.f), Colors::White);
}

void HUD::ShowDeathMessage(uint32_t inPlayerId)
{
	string playerName = StringUtils::Sprintf("Player %d", inPlayerId);

	ScoreBoardManager::Entry* entry =
		ScoreBoardManager::sInstance->GetEntry(inPlayerId);

	if (entry)
	{
		playerName = entry->GetPlayerName();
	}

	mDeathMessage = playerName + " exploded!";
	mDeathMessageTimer = 3.f;
}

void HUD::RenderDeathMessage()
{
	if (mDeathMessageTimer <= 0.f)
	{
		return;
	}

	mDeathMessageTimer -= Timing::sInstance.GetDeltaTime();

	sf::RectangleShape overlay;
	overlay.setSize(sf::Vector2f(1920.f, 1080.f));
	overlay.setPosition(0.f, 0.f);
	overlay.setFillColor(sf::Color(80, 80, 80, 120));

	WindowManager::sInstance->draw(overlay);

	sf::Text text;
	text.setString(mDeathMessage);
	text.setFont(*FontManager::sInstance->GetFont("carlito"));
	text.setCharacterSize(64);
	text.setFillColor(sf::Color::White);

	sf::FloatRect bounds = text.getLocalBounds();

	text.setOrigin(
		bounds.left + bounds.width / 2.f,
		bounds.top + bounds.height / 2.f
	);

	text.setPosition(960.f, 540.f);

	WindowManager::sInstance->draw(text);
}

