#include <STDInclude.hpp>

namespace Utils::Maths
{
	float DotProduct(float v1[3], float v2[3])
	{
		return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
	}

	void VectorSubtract(const float va[3], const float vb[3], float out[3])
	{
		out[0] = va[0] - vb[0];
		out[1] = va[1] - vb[1];
		out[2] = va[2] - vb[2];
	}

	void VectorAdd(float va[3], float vb[3], float out[3])
	{
		out[0] = va[0] + vb[0];
		out[1] = va[1] + vb[1];
		out[2] = va[2] + vb[2];
	}

	void VectorCopy(float in[3], float out[3])
	{
		out[0] = in[0];
		out[1] = in[1];
		out[2] = in[2];
	}

	void VectorScale(float v[3], float scale, float out[3])
	{
		out[0] = v[0] * scale;
		out[1] = v[1] * scale;
		out[2] = v[2] * scale;
	}

	float Vec3SqrDistance(const float v1[3], const float v2[3])
	{
		float out[3];

		VectorSubtract(v2, v1, out);

		return (out[0] * out[0]) + (out[1] * out[1]) + (out[2] * out[2]);
	}
}
