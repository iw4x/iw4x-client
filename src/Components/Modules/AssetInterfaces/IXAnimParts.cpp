#include <STDInclude.hpp>

namespace Assets
{
	void IXAnimParts::Mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::XAnimParts* asset = header.xanim;

		if (asset->tagnames)
		{
			for (char i = 0; i < asset->boneCount[Game::XAnimPartType::PART_TYPE_ALL]; ++i)
			{
				builder->AddScriptString(asset->tagnames[i]);
			}
		}

		if (asset->notetracks)
		{
			for (char i = 0; i < asset->notetrackCount; ++i)
			{
				builder->AddScriptString(asset->notetracks[i].name);
			}
		}
	}

	void IXAnimParts::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::XAnimParts, 88);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::XAnimParts* asset = header.xanim;
		Game::XAnimParts* dest = buffer->Dest<Game::XAnimParts>();
		buffer->Save(asset, sizeof(Game::XAnimParts));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = reinterpret_cast<char*>(-1);
		}

		if (asset->tagnames)
		{
			buffer->Align(Utils::Stream::ALIGN_2);

			unsigned short* destTagnames = buffer->Dest<unsigned short>();
			buffer->SaveArray(asset->tagnames, asset->boneCount[Game::XAnimPartType::PART_TYPE_ALL]);

			for (char i = 0; i < asset->boneCount[Game::XAnimPartType::PART_TYPE_ALL]; ++i)
			{
				builder->MapScriptString(&destTagnames[i]);
			}

			dest->tagnames = reinterpret_cast<short*>(-1);
		}

		if (asset->notetracks)
		{
			Assert_Size(Game::XAnimNotifyInfo, 8);
			buffer->Align(Utils::Stream::ALIGN_4);

			Game::XAnimNotifyInfo* destNotetracks = buffer->Dest<Game::XAnimNotifyInfo>();
			buffer->SaveArray(asset->notetracks, asset->notetrackCount);

			for (char i = 0; i < asset->notetrackCount; ++i)
			{
				builder->MapScriptString(&destNotetracks[i].name);
			}

			dest->notetracks = reinterpret_cast<Game::XAnimNotifyInfo*>(-1);
		}

		if (asset->delta)
		{
			Assert_Size(Game::XAnimDeltaPart, 12);
			buffer->Align(Utils::Stream::ALIGN_4);

			dest->delta = reinterpret_cast<Game::XAnimDeltaPart*>(-1);
		}

		if (asset->dataByte)
		{
			buffer->SaveArray(asset->dataByte, asset->dataByteCount);
			dest->dataByte = reinterpret_cast<char*>(-1);
		}

		if (asset->dataShort)
		{
			buffer->Align(Utils::Stream::ALIGN_2);
			buffer->SaveArray(asset->dataShort, asset->dataShortCount);
			dest->dataShort = reinterpret_cast<short*>(-1);
		}

		if (asset->dataInt)
		{
			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->SaveArray(asset->dataInt, asset->dataIntCount);
			dest->dataInt = reinterpret_cast<int*>(-1);
		}

		if (asset->randomDataShort)
		{
			buffer->Align(Utils::Stream::ALIGN_2);
			buffer->SaveArray(asset->randomDataShort, asset->randomDataShortCount);
			dest->randomDataShort = reinterpret_cast<short*>(-1);
		}

		if (asset->randomDataByte)
		{
			buffer->SaveArray(asset->randomDataByte, asset->randomDataByteCount);
			dest->randomDataByte = reinterpret_cast<char*>(-1);
		}

		if (asset->randomDataInt)
		{
			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->SaveArray(asset->randomDataInt, asset->randomDataIntCount);
			dest->randomDataInt = reinterpret_cast<int*>(-1);
		}

		if (asset->indices.data)
		{
			if (asset->framecount > 0xFF)
			{
				buffer->Align(Utils::Stream::ALIGN_2);
				buffer->SaveArray(dest->indices._2, asset->framecount);
			}
			else
			{
				buffer->SaveArray(dest->indices._1, asset->framecount);
			}

			dest->indices.data = reinterpret_cast<void*>(-1);
		}

		buffer->PopBlock();
	}
}
