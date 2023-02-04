#include <STDInclude.hpp>
#include <json.hpp>

#include "ISndCurve.hpp"

namespace Assets
{
	void ISndCurve::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::SndCurve, 136);

		auto* buffer = builder->getBuffer();
		auto* asset = header.sndCurve;
		auto* dest = buffer->dest<Game::SndCurve>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->filename)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->filename));
			Utils::Stream::ClearPointer(&dest->filename);
		}

		buffer->popBlock();
	}

	void ISndCurve::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File sndCurveFile(std::format("sndcurve/{}.iw4x.json", name));

		if (!sndCurveFile.exists())
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Missing file for sndcurve {}!", name);
			return;
		}

		nlohmann::json sndCurveJson;
		try
		{
			sndCurveJson = nlohmann::json::parse(sndCurveFile.getBuffer());
		}
		catch (const std::exception& e)
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Invalid JSON for sndcurve {}! {}", name, e.what());
			return;
		}

		auto* sndCurve = builder->getAllocator()->allocate<Game::SndCurve>();
		try
		{
			sndCurve->filename = builder->getAllocator()->duplicateString(sndCurveJson["filename"].get<std::string>());
			sndCurve->knotCount = sndCurveJson["knotCount"].get<unsigned short>();

			for (auto side = 0; side < 2; side++)
			{
				for (auto knot = 0; knot < 16; knot++)
				{
					sndCurve->knots[knot][side] = sndCurveJson["knots"][knot][side].get<float>();
				}
			}
		}
		catch (const std::exception& e)
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Malformed JSON for sndcurve {}! {}", name, e.what());
			return;
		}

		header->sndCurve = sndCurve;
	}
}
