namespace Utils
{
	namespace Compression
	{
		class ZLib
		{
		public:
			static std::string Compress(std::string data);
			static std::string Decompress(std::string data);
		};
	};
}
