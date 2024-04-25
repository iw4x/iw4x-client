#include <STDInclude.hpp>
#include "HuffmanTree.hpp"

namespace Utils::Huffman
{
	using namespace Utils::Huffman::Tree;

	int Decompress(const unsigned char* from, unsigned char* to, int fromSize, int toSize)
	{
		const std::span input(from, fromSize);
		const std::span output(to, toSize);
		int outputByteCount = 0;

		for (std::size_t inputBitCount = 0; inputBitCount < input.size() * 8 && outputByteCount < std::ssize(output); ++outputByteCount)
		{
			std::size_t nodeIndex = decompressionData.size() - 1;

			do
			{
				const bool rightNode = (input[inputBitCount / 8] >> (inputBitCount % 8)) & 1;
				++inputBitCount;

				nodeIndex = (rightNode) ? decompressionData[nodeIndex].right : decompressionData[nodeIndex].left;
			}
			while (decompressionData[nodeIndex].symbol == invalidNode);

			assert(("Node index must be equal to symbol value!" && nodeIndex == decompressionData[nodeIndex].symbol));
			output[outputByteCount] = static_cast<std::uint8_t>(decompressionData[nodeIndex].symbol);
		}

		return outputByteCount;
	}

	int Compress(const unsigned char* from, unsigned char* to, int fromSize, int toSize)
	{
		const std::span input(from, fromSize);
		const std::span output(to, toSize);
		int outputBitCount = 0;

		for (const auto byte : input)
		{
			const std::size_t nodeCount = compressionData[byte].nodeData.front(); // get bit count

			for (std::size_t nodeIndex = 1; nodeIndex < nodeCount + 1; ++nodeIndex)
			{
				if (outputBitCount % 8 == 0) // beginning of a new byte
				{
					output[outputBitCount / 8] = static_cast<std::uint8_t>(compressionData[byte].nodeData[nodeIndex] << (outputBitCount % 8));
				}
				else
				{
					output[outputBitCount / 8] |= static_cast<std::uint8_t>(compressionData[byte].nodeData[nodeIndex] << (outputBitCount % 8));
				}

				++outputBitCount;
			}
		};

		return (outputBitCount + 7) / 8;
	}
}
