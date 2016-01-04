#define XFILE_MAGIC_UNSIGNED 0x3030317566665749
#define XFILE_VERSION 276

#pragma pack(push, 1)
struct  XFileHeader
{
	uint64_t magic;
	uint32_t version;
	uint8_t flag;
	DWORD highDateTime;
	DWORD lowDateTime;
};
#pragma pack(pop)

struct XFile
{
	unsigned int size;
	unsigned int externalSize;
	unsigned int blockSize[8];
};

struct ScriptStringList
{
	int count;
	const char **strings;
};

struct XAssetList
{
	ScriptStringList stringList;
	int assetCount;
	Game::XAsset *assets;
};

namespace Components
{
	class ZoneBuilder : public Component
	{
	public:
		class Zone
		{
		public:
			Zone(std::string zoneName);
			~Zone();

			std::string Build();

		private:
			std::string ZoneName;
		};

		ZoneBuilder();
		const char* GetName() { return "ZoneBuilder"; };

		static bool IsEnabled();
	};
}
