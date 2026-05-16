//Joseph Byrne D00255161

#pragma once
class PlayerSpriteComponent : public SpriteComponent
{
public:
	PlayerSpriteComponent(GameObject* inGameObject);
	virtual sf::Sprite& GetSprite() override;

private:
	string GetDirectionKey(float inRotation, bool inHasPotato) const;
};
typedef shared_ptr<PlayerSpriteComponent >	PlayerSpriteComponentPtr;

