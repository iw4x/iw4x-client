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

	void IXAnimParts::Save_XAnimDeltaPart(Game::XAnimDeltaPart* delta, unsigned short framecount, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::XAnimDeltaPart, 12);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::XAnimDeltaPart* destDelta = buffer->Dest<Game::XAnimDeltaPart>();
		buffer->Save(delta, sizeof(Game::XAnimDeltaPart));

		if (delta->trans)
		{
			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->Save(delta->trans, 4);

			if (delta->trans->size)
			{
				buffer->Save(&delta->trans->u.frames, 28);

				if (framecount > 0xFF)
				{
					buffer->SaveArray(delta->trans->u.frames.indices._2, delta->trans->size + 1);
				}
				else
				{
					buffer->SaveArray(delta->trans->u.frames.indices._1, delta->trans->size + 1);
				}

				if (delta->trans->u.frames.frames._1)
				{
					if (delta->trans->smallTrans)
					{
						buffer->Save(delta->trans->u.frames.frames._1, 3, delta->trans->size + 1);
					}
					else
					{
						buffer->Align(Utils::Stream::ALIGN_4);
						buffer->Save(delta->trans->u.frames.frames._1, 6, delta->trans->size + 1);
					}
				}
			}
			else
			{
				buffer->Save(delta->trans->u.frame0, 12);
			}

			destDelta->trans = reinterpret_cast<Game::XAnimPartTrans*>(-1);
		}

		if (delta->quat2)
		{
			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->Save(delta->quat2, 4);

			if (delta->quat2->size)
			{
				buffer->Save(&delta->quat2->u.frames, 4);

				if (framecount > 0xFF)
				{
					buffer->Save(delta->quat2->u.frames.indices, 2, delta->quat2->size + 1);
				}
				else
				{
					buffer->Save(delta->quat2->u.frames.indices, 1, delta->quat2->size + 1);
				}

				if (delta->quat2->u.frames.frames)
				{
					buffer->Align(Utils::Stream::ALIGN_4);
					buffer->Save(delta->quat2->u.frames.frames, 4, delta->quat2->size + 1);
				}
			}
			else
			{
				buffer->Save(delta->quat2->u.frame0, 4);
			}

			destDelta->quat2 = reinterpret_cast<Game::XAnimDeltaPartQuat2*>(-1);
		}

		if (delta->quat)
		{
			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->Save(delta->quat, 4);

			if (delta->quat->size)
			{
				buffer->Save(&delta->quat->u.frames, 4);

				if (framecount > 0xFF)
				{
					buffer->Save(delta->quat->u.frames.indices, 2, delta->quat->size + 1);
				}
				else
				{
					buffer->Save(delta->quat->u.frames.indices, 1, delta->quat->size + 1);
				}

				if (delta->quat->u.frames.frames)
				{
					buffer->Align(Utils::Stream::ALIGN_4);
					buffer->Save(delta->quat->u.frames.frames, 4, delta->quat->size + 1);
				}
			}
			else
			{
				buffer->Save(delta->quat->u.frame0, 4);
			}

			destDelta->quat = reinterpret_cast<Game::XAnimDeltaPartQuat*>(-1);
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

			IXAnimParts::Save_XAnimDeltaPart(asset->delta, asset->framecount, builder);

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
