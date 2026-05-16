#pragma once

class ExplosionEffect : public GameObject
{
public:
	ExplosionEffect(const Vector3& inLocation);
	virtual void Update() override;


private:
	float mTimer;
	int mCurrentFrame;
	SpriteComponentPtr mSprite;
};