#include "RoboCatPCH.hpp"
//Eoin Hamill D00258444
namespace
{
	void WriteSignedBinaryValue(OutputMemoryBitStream& inOutputStream, float inValue)
	{
		bool isNonZero = (inValue != 0.f);
		inOutputStream.Write(isNonZero);
		if (isNonZero)
		{
			inOutputStream.Write(inValue > 0.f);
		}
	}

	void ReadSignedBinaryValue(InputMemoryBitStream& inInputStream, float& outValue)
	{
		bool isNonZero;
		inInputStream.Read(isNonZero);
		if (isNonZero)
		{
			bool isPositive;
			inInputStream.Read(isPositive);
			outValue = isPositive ? 1.f : -1.f;
		}
		else
		{
			outValue = 0.f;
		}
	}
}

bool InputState::Write(OutputMemoryBitStream& inOutputStream) const
{
	//writes you horizontal movement into the packet
	WriteSignedBinaryValue(inOutputStream, GetDesiredHorizontalDelta());
	//write vertical movement to a packet
	WriteSignedBinaryValue(inOutputStream, GetDesiredVerticalDelta());
	inOutputStream.Write(mIsShooting);
	//write dash to server
	inOutputStream.Write(mIsDashing);

	return false;
}

bool InputState::Read(InputMemoryBitStream& inInputStream)
{
	//Read the movements sent over the network
	ReadSignedBinaryValue(inInputStream, mDesiredRightAmount);
	ReadSignedBinaryValue(inInputStream, mDesiredForwardAmount);
	inInputStream.Read(mIsShooting);
	inInputStream.Read(mIsDashing);

	return true;
}