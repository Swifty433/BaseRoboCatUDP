#pragma once

#include "GameObject.hpp"

class ExplosionEffect : public GameObject
{
public:
	ExplosionEffect(const Vector3& inLocation);
		virtual void Update() override;

private:
	float mTimer;
	int mCurrentFrame;
};