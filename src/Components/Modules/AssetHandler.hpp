namespace Components
{
	class AssetHandler : public Component
	{
	public:
		class IAsset
		{
		public:
			virtual Game::XAssetType GetType() { return Game::XAssetType::ASSET_TYPE_INVALID; };
			virtual void Mark(Game::XAssetHeader header, ZoneBuilder::Zone* builder) { /*ErrorTypeNotSupported(this);*/ };
			virtual void Save(Game::XAssetHeader header, ZoneBuilder::Zone* builder) { /*ErrorTypeNotSupported(this);*/ };
			virtual void Dump(Game::XAssetHeader header) { /*ErrorTypeNotSupported(this);*/ };
			virtual void Load(Game::XAssetHeader* header, std::string name, ZoneBuilder::Zone* builder) { /*ErrorTypeNotSupported(this);*/ };
		};

		typedef Game::XAssetHeader(*Callback)(Game::XAssetType, const char*);
		typedef bool(*RestrictCallback)(Game::XAssetType type, Game::XAssetHeader asset, const char* name);

		AssetHandler();
		~AssetHandler();
		const char* GetName() { return "AssetHandler"; };

		static void OnFind(Game::XAssetType type, Callback callback);
		static void OnLoad(RestrictCallback callback);

		static void Relocate(void* start, void* to, DWORD size = 4);

		static void ZoneSave(Game::XAsset asset, ZoneBuilder::Zone* builder);
		static void ZoneMark(Game::XAsset asset, ZoneBuilder::Zone* builder);

		static Game::XAssetHeader FindOriginalAsset(Game::XAssetType type, const char* filename);
		static Game::XAssetHeader FindAssetForZone(Game::XAssetType type, std::string filename, ZoneBuilder::Zone* builder);

		static void ClearTemporaryAssets();
		static void StoreTemporaryAsset(Game::XAssetType type, Game::XAssetHeader asset);

	private:
		static bool BypassState;

		static void RegisterInterface(IAsset* iAsset);

		static Game::XAssetHeader FindAsset(Game::XAssetType type, const char* filename);
		static bool IsAssetEligible(Game::XAssetType type, Game::XAssetHeader* asset);
		static void FindAssetStub();
		static void AddAssetStub();

		static void OffsetToAlias(Utils::Stream::Offset* offset);

		static std::map<std::string, Game::XAssetHeader> TemporaryAssets[Game::XAssetType::ASSET_TYPE_COUNT];

		static std::map<Game::XAssetType, IAsset*> AssetInterfaces;
		static std::map<Game::XAssetType, Callback> TypeCallbacks;
		static std::vector<RestrictCallback> RestrictCallbacks;

		static std::map<void*, void*> Relocations;
	};
}

#include "AssetInterfaces\IXModel.hpp"
#include "AssetInterfaces\IRawFile.hpp"
#include "AssetInterfaces\IGfxImage.hpp"
#include "AssetInterfaces\IMaterial.hpp"
#include "AssetInterfaces\IPhysPreset.hpp"
#include "AssetInterfaces\IPhysCollmap.hpp"
#include "AssetInterfaces\IXModelSurfs.hpp"
#include "AssetInterfaces\ILocalizedEntry.hpp"
#include "AssetInterfaces\IMaterialPixelShader.hpp"
#include "AssetInterfaces\IMaterialTechniqueSet.hpp"
#include "AssetInterfaces\IMaterialVertexShader.hpp"
#include "AssetInterfaces\IMaterialVertexDeclaration.hpp"
