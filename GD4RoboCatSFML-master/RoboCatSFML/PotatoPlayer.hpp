//Joseph Byrne D00255161

#pragma once
#include "GameObject.hpp"

class PotatoPlayer : public GameObject
{
	public:
		CLASS_IDENTIFICATION('PTOP', PotatoPlayer)

		enum EPotatoReplicationState
		{
			EPRS_Pose = 1 << 0,
			EPRS_Color = 1 << 1,
			EPRS_PlayerId = 1 << 2,
			EPRS_Health = 1 << 3,
			EPRS_Potato = 1 << 4,

			EPRS_AllState = EPRS_Pose | EPRS_Color | EPRS_PlayerId | EPRS_Health | EPRS_Potato
		};

		static GameObject* StaticCreate() { return new PotatoPlayer(); }

		virtual uint32_t GetAllStateMask() const override { return EPRS_AllState; }
		virtual PotatoPlayer* GetAsPotatoPlayer() { return this; }

		virtual void Update() override;

		void ProcessInput(float inDeltaTime, const InputState& inInputState);
		void SimulateMovement(float inDeltaTime);
		void ProcessCollisions();
		void ProcessCollisionsWithScreenWalls();

		void 
};

