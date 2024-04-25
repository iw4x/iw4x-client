#pragma once

namespace Components
{
	class Huffman : public Component
	{
	public:
		Huffman();

		static int MSG_ReadBitsCompressStub();
		static int MSG_WriteBitsCompressStub();

		bool unitTest() override;
	};
}
