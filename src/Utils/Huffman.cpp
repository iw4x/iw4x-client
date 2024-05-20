#include <STDInclude.hpp>
#include "HuffmanTree.hpp"

namespace Utils::Huffman
{
	using namespace Utils::Huffman::Tree;

	int Decompress(const unsigned char* from, unsigned char* to, int fromSize, int toSize)
	{
		if (fromSize <= 0)
		{
			Components::Logger::Print("Huffman decompression expects an input buffer size bigger than {}\n", fromSize);
			return 0;
		}
		if (toSize <= 0)
		{
			Components::Logger::Print("Huffman decompression expects an output buffer size bigger than {}\n", toSize);
			return 0;
		}

		const std::span input(from, fromSize);
		const std::span output(to, toSize);
		std::size_t outputByteCount = 0;

		for (std::size_t inputBitCount = 0; inputBitCount < input.size() * 8 && outputByteCount < output.size(); ++outputByteCount)
		{
			[[maybe_unused]] const std::size_t orgInputBitCount = inputBitCount;
			std::size_t nodeIndex = decompressionData.size() - 1;

			do
			{
				const bool rightNode = (input[inputBitCount / 8] >> (inputBitCount % 8)) & 1;
				if (++inputBitCount >= input.size() * 8)
				{
					// some symbols take more than 8 bits to (de)compress, so the check in the outer loop isn't adequate to prevent OOB in the inner loop
					Components::Logger::Debug("Huffman decompression out-of-bounds read detected!");
					return static_cast<int>(outputByteCount);
				}

				assert((inputBitCount - orgInputBitCount < 12 && "No symbol should take more than 11 bits to decompress!"));
				nodeIndex = (rightNode) ? decompressionData[nodeIndex % 256].right : decompressionData[nodeIndex % 256].left;
			}
			while (nodeIndex >= 256);

			output[outputByteCount] = static_cast<std::uint8_t>(nodeIndex);
		}

		return static_cast<int>(outputByteCount);
	}

	int Compress(const unsigned char* from, unsigned char* to, int fromSize, int toSize)
	{
		if (fromSize <= 0)
		{
			Components::Logger::Print("Huffman compression expects an input buffer size bigger than {}\n", fromSize);
			return 0;
		}
		if (toSize <= 0)
		{
			Components::Logger::Print("Huffman compression expects an output buffer size bigger than {}\n", toSize);
			return 0;
		}

		const std::span input(from, fromSize);
		const std::span output(to, toSize);
		std::size_t outputBitCount = 0;

		for (std::size_t inputByteCount = 0; inputByteCount < input.size() && outputBitCount < output.size() * 8; ++inputByteCount)
		{
			const std::uint8_t byte = input[inputByteCount];
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

				if (++outputBitCount >= output.size() * 8)
				{
					// some symbols take more than 8 bits to (de)compress, so the check in the outer loop isn't adequate to prevent OOB in the inner loop
					Components::Logger::Debug("Huffman compression out-of-bounds write detected!");
					return static_cast<int>((outputBitCount + 7) / 8);
				}
			}
		};

		return static_cast<int>((outputBitCount + 7) / 8);
	}
}
