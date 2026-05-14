// Joseph Byrne D00255161

#include "RoboCatClientPCH.hpp"

PlayerSpriteComponent::PlayerSpriteComponent(GameObject* inGameObject) :
    SpriteComponent(inGameObject)
{
}

sf::Sprite& PlayerSpriteComponent::GetSprite()
{
	// Update the sprite based on the game object stuff.
    auto pos = mGameObject->GetLocation();
    auto rot = mGameObject->GetRotation();
    m_sprite.setPosition(pos.mX, pos.mY);
    m_sprite.setRotation(rot);

    PotatoPlayer* player = dynamic_cast<PotatoPlayer*>(mGameObject);
    if (player)
    {
        Vector3 playerColor = player->GetColor();
        m_sprite.setColor(sf::Color(
            static_cast<sf::Uint8>(playerColor.mX),
            static_cast<sf::Uint8>(playerColor.mY),
            static_cast<sf::Uint8>(playerColor.mZ),
            255));
    }

    return m_sprite;
}