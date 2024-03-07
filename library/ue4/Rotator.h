#pragma once

struct FRotator
{
	float Pitch, Yaw, Roll;

	FRotator()
		: Pitch(0), Yaw(0), Roll(0)
	{ }

	FRotator(float pitch, float yaw, float roll) : Pitch(pitch), Yaw(yaw), Roll(roll) {}

	FRotator operator+ (const FRotator& other) const { return FRotator(Pitch + other.Pitch, Yaw + other.Yaw, Roll + other.Roll); }

	FRotator operator- (const FRotator& other) const { return FRotator(Pitch - other.Pitch, Yaw - other.Yaw, Roll - other.Roll); }

	FRotator operator* (float scalar) const { return FRotator(Pitch * scalar, Yaw * scalar, Roll * scalar); }

	FRotator& operator=  (const FRotator& other) { Pitch = other.Pitch; Yaw = other.Yaw; Roll = other.Roll; return *this; }

	FRotator& operator+= (const FRotator& other) { Pitch += other.Pitch; Yaw += other.Yaw; Roll += other.Roll; return *this; }

	FRotator& operator-= (const FRotator& other) { Pitch -= other.Pitch; Yaw -= other.Yaw; Roll -= other.Roll; return *this; }

	FRotator& operator*= (const float other) { Yaw *= other; Pitch *= other; Roll *= other; return *this; }

	FRotator Clamp()
	{
		if (Pitch > 180)
			Pitch -= 360;
		else if (Pitch < -180)
			Pitch += 360;
		if (Yaw > 180)
			Yaw -= 360;
		else if (Yaw < -180)
			Yaw += 360;
		if (Pitch < -89)
			Pitch = -89;
		if (Pitch > 89)
			Pitch = 89;
		while (Yaw < -180.0f)
			Yaw += 360.0f;
		while (Yaw > 180.0f)
			Yaw -= 360.0f;
		Roll = 0;
		return *this;
	}

	struct FQuat Quaternion() const;
};