#include "HuffmanTree.hpp"

namespace Utils::Huffman
{
	using namespace Utils::Huffman::Tree;

	int Compress(const unsigned char* input, unsigned char* output, int inputSize, int outputSize)
	{
		int outputBitCount = 0;

		for (int inputByteCount = 0; inputByteCount < inputSize && outputBitCount < outputSize * 8; ++inputByteCount)
		{
			const auto byte = input[inputByteCount];
			const auto nodeCount = compressionData[byte].nodeData.front(); // get bit count

			for (unsigned int nodeIndex = 1; nodeIndex <= nodeCount; ++nodeIndex)
			{
				if ((outputBitCount & 7) == 0) // beginning of a new byte
				{
					output[outputBitCount / 8] = static_cast<unsigned char>(compressionData[byte].nodeData[nodeIndex] << (outputBitCount & 7));
				}
				else
				{
					output[outputBitCount / 8] |= static_cast<unsigned char>(compressionData[byte].nodeData[nodeIndex] << (outputBitCount & 7));
				}

				if (++outputBitCount >= outputSize * 8)
				{
					// some symbols take more than 8 bits to (de)compress, so the check in the outer loop isn't adequate to prevent OOB in the inner loop
					break;
				}
			}
		}

		return (outputBitCount + 7) / 8;
	}

	int Decompress(const unsigned char* input, unsigned char* output, int inputSize, int outputSize)
	{
		int outputByteCount = 0;

		for (int inputBitCount = 0; inputBitCount < inputSize * 8 && outputByteCount < outputSize; ++outputByteCount)
		{
			[[maybe_unused]] const auto orgInputBitCount = inputBitCount;
			auto nodeIndex = decompressionData.size() - 1;

			do
			{
				const bool rightNode = (input[inputBitCount / 8] >> (inputBitCount & 7)) & 1;
				nodeIndex = (rightNode) ? decompressionData[nodeIndex % 256].right : decompressionData[nodeIndex % 256].left;

				assert((inputBitCount + 1 - orgInputBitCount < 12 && "No symbol should take more than 11 bits to decompress!"));
				if (++inputBitCount >= inputSize * 8)
				{
					// some symbols take more than 8 bits to (de)compress, so the check in the outer loop isn't adequate to prevent OOB in the inner loop
					break;
				}
			}
			while (nodeIndex >= 256);

			output[outputByteCount] = static_cast<unsigned char>(nodeIndex);
		}

		return outputByteCount;
	}
}
