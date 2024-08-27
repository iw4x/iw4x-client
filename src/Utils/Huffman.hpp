#pragma once

namespace Utils::Huffman
{
	[[nodiscard]] int Compress(const unsigned char* input, unsigned char* output, int inputSize, int outputSize);
	[[nodiscard]] int Decompress(const unsigned char* input, unsigned char* output, int inputSize, int outputSize);
}
