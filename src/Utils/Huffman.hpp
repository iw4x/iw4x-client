#pragma once

namespace Utils::Huffman
{
	[[nodiscard]] int Compress(const unsigned char* from, unsigned char* to, int fromSize, int toSize);
	[[nodiscard]] int Decompress(const unsigned char* from, unsigned char* to, int fromSize, int toSize);
}
