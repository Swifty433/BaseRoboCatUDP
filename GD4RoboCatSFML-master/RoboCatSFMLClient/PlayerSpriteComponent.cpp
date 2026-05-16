// Joseph Byrne D00255161

#include "RoboCatClientPCH.hpp"

PlayerSpriteComponent::PlayerSpriteComponent(GameObject* inGameObject) :
    SpriteComponent(inGameObject)
{
}

string PlayerSpriteComponent::GetDirectionKey(float inRotation, bool inHasPotato) const
{
	inRotation -= 90.f; // Adjust so 0 is up, 90 is right, etc.

    while (inRotation < 0.f)    inRotation += 360.f;
    while (inRotation >= 360.f) inRotation -= 360.f;

    string direction;

    if (inRotation >= 337.5f || inRotation < 22.5f)  direction = "right";
    else if (inRotation >= 22.5f && inRotation < 67.5f)  direction = "downright";
    else if (inRotation >= 67.5f && inRotation < 112.5f) direction = "down";
    else if (inRotation >= 112.5f && inRotation < 157.5f) direction = "downleft";
    else if (inRotation >= 157.5f && inRotation < 202.5f) direction = "left";
    else if (inRotation >= 202.5f && inRotation < 247.5f) direction = "upleft";
    else if (inRotation >= 247.5f && inRotation < 292.5f) direction = "up";
    else                                                    direction = "upright";

    string prefix = inHasPotato ? "player_potato_" : "player_";
    return prefix + direction;
}

sf::Sprite& PlayerSpriteComponent::GetSprite()
{
	// Update the sprite based on the game object stuff.
    auto pos = mGameObject->GetLocation();
    //auto rot = mGameObject->GetRotation();
    m_sprite.setPosition(pos.mX, pos.mY);
    //m_sprite.setRotation(rot);

    PotatoPlayer* player = dynamic_cast<PotatoPlayer*>(mGameObject);
    if (player)
    {
		//picking the right directional texture based on rotation and potato status
		string key = GetDirectionKey(player->GetRotation(), player->HasPotato());
		TexturePtr tex = TextureManager::sInstance->GetTexture(key);
        if (tex)
        {
			auto tSize = tex->getSize();
			m_sprite.setTexture(*tex);
			m_sprite.setOrigin(tSize.x / 2.f, tSize.y / 2.f);
			m_sprite.setScale(mGameObject->GetScale(), mGameObject->GetScale());
        }

        //applying colour
        Vector3 playerColor = player->GetColor();
        m_sprite.setColor(sf::Color(
            static_cast<sf::Uint8>(playerColor.mX),
            static_cast<sf::Uint8>(playerColor.mY),
            static_cast<sf::Uint8>(playerColor.mZ),
            255));
    }

    return m_sprite;
}