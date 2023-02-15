#pragma once

namespace Utils::Maths
{
	// Macros in Quake-III
	constexpr auto VectorClear(float x[3]) { x[0] = x[1] = x[2] = 0; }
	constexpr auto VectorNegate(float x[3]) { x[0] = -x[0]; x[1] = -x[1]; x[2] = -x[2]; }

	float DotProduct(float v1[3], float v2[3]);
	void VectorSubtract(const float va[3], const float vb[3], float out[3]);
	void VectorAdd(float va[3], float vb[3], float out[3]);
	void VectorCopy(float in[3], float out[3]);
	void VectorScale(float v[3], float scale, float out[3]);
	float Vec3SqrDistance(const float v1[3], const float v2[3]);
}
