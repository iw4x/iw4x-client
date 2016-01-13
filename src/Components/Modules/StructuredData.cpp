#include "STDInclude.hpp"

namespace Components
{
	void StructuredData::DumpDataDef(Game::structuredDataDef_t* dataDef)
	{
		if (!dataDef || !dataDef->data) return;

		json11::Json definition =
		json11::Json::object
		{
			{ "version", dataDef->data->version },
			//{ "enums", [ 0 ] },
		};

		Utils::WriteFile(Utils::VA("raw/%s.json", dataDef->name), definition.dump());
	}

	StructuredData::StructuredData()
	{
		Command::Add("dumpDataDef", [] (Command::Params params)
		{
			if (params.Length() < 2) return;
			StructuredData::DumpDataDef(Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_STRUCTUREDDATADEF, params[1]).structuredData);
		});
	}

	StructuredData::~StructuredData()
	{

	}
}
