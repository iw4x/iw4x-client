#pragma once

namespace Components
{
	class Huffman : public Component
	{
	public:
		Huffman();

		bool unitTest() override;

		static inline bool isInitialized;
	};
}
