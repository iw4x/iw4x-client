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
			[[maybe_unused]] const std::size_t orgInputBitCount = inputBitCount;
			std::size_t nodeIndex = decompressionData.size() - 1;

			do
			{
				const bool rightNode = (input[inputBitCount / 8] >> (inputBitCount % 8)) & 1;
				++inputBitCount;

				assert((inputBitCount - orgInputBitCount < 12 && "No symbol should take more than 11 bits to decompress!"));
				nodeIndex = (rightNode) ? decompressionData[nodeIndex % 256].right : decompressionData[nodeIndex % 256].left;
			}
			while (nodeIndex >= 256);

			output[outputByteCount] = static_cast<std::uint8_t>(nodeIndex);
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
