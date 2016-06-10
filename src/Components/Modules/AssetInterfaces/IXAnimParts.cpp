#include <STDInclude.hpp>

namespace Assets
{
	void IXAnimParts::Load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File animFile(Utils::VA("xanim/%s.iw4xAnim", name.data()));

		if (animFile.Exists())
		{
			Utils::Stream::Reader reader(builder->GetAllocator(), animFile.GetBuffer());

			Game::XAnimParts* xanim = reader.ReadArray<Game::XAnimParts>();

			if (xanim)
			{
				if (xanim->name)
				{
					xanim->name = reader.ReadCString();
				}

				if (xanim->tagnames)
				{
					xanim->tagnames = builder->GetAllocator()->AllocateArray<short>(xanim->boneCount[Game::XAnimPartType::PART_TYPE_ALL]);
					for (int i = 0; i < xanim->boneCount[Game::XAnimPartType::PART_TYPE_ALL]; ++i)
					{
						xanim->tagnames[i] = Game::SL_GetString(reader.ReadCString(), 0);
					}
				}

				if (xanim->notetracks)
				{
					xanim->notetracks = reader.ReadArray<Game::XAnimNotifyInfo>(xanim->notetrackCount);

					for (int i = 0; i < xanim->notetrackCount; ++i)
					{
						xanim->notetracks[i].name = Game::SL_GetString(reader.ReadCString(), 0);
					}
				}

				if (xanim->dataByte)
				{
					xanim->dataByte = reader.ReadArray<char>(xanim->dataByteCount);
				}

				if (xanim->dataShort)
				{
					xanim->dataShort = reader.ReadArray<short>(xanim->dataShortCount);
				}

				if (xanim->dataInt)
				{
					xanim->dataInt = reader.ReadArray<int>(xanim->dataIntCount);
				}

				if (xanim->randomDataByte)
				{
					xanim->randomDataByte = reader.ReadArray<char>(xanim->randomDataByteCount);
				}

				if (xanim->randomDataShort)
				{
					xanim->randomDataShort = reader.ReadArray<short>(xanim->randomDataShortCount);
				}

				if (xanim->randomDataInt)
				{
					xanim->randomDataInt = reader.ReadArray<int>(xanim->randomDataIntCount);
				}

				if (xanim->indices.data)
				{
					if (xanim->framecount < 256)
					{
						xanim->indices._1 = reader.ReadArray<char>(xanim->indexcount);
					}
					else
					{
						xanim->indices._2 = reader.ReadArray<unsigned short>(xanim->indexcount);
					}
				}

				if (!reader.End())
				{
					Components::Logger::Error(0, "Reading animation '%s' failed, remaining raw data found!", name.data());
				}

				header->xanim = xanim;
			}
		}
	}

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

			Utils::Stream::ClearPointer(&destDelta->trans);
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

			Utils::Stream::ClearPointer(&destDelta->quat2);
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

			Utils::Stream::ClearPointer(&destDelta->quat);
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
			Utils::Stream::ClearPointer(&dest->name);
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

			Utils::Stream::ClearPointer(&dest->tagnames);
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

			Utils::Stream::ClearPointer(&dest->notetracks);
		}

		if (asset->delta)
		{
			Assert_Size(Game::XAnimDeltaPart, 12);
			buffer->Align(Utils::Stream::ALIGN_4);

			IXAnimParts::Save_XAnimDeltaPart(asset->delta, asset->framecount, builder);

			Utils::Stream::ClearPointer(&dest->delta);
		}

		if (asset->dataByte)
		{
			buffer->SaveArray(asset->dataByte, asset->dataByteCount);
			Utils::Stream::ClearPointer(&dest->delta);
		}

		if (asset->dataShort)
		{
			buffer->Align(Utils::Stream::ALIGN_2);
			buffer->SaveArray(asset->dataShort, asset->dataShortCount);
			Utils::Stream::ClearPointer(&dest->dataShort);
		}

		if (asset->dataInt)
		{
			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->SaveArray(asset->dataInt, asset->dataIntCount);
			Utils::Stream::ClearPointer(&dest->dataInt);
		}

		if (asset->randomDataShort)
		{
			buffer->Align(Utils::Stream::ALIGN_2);
			buffer->SaveArray(asset->randomDataShort, asset->randomDataShortCount);
			Utils::Stream::ClearPointer(&dest->randomDataShort);
		}

		if (asset->randomDataByte)
		{
			buffer->SaveArray(asset->randomDataByte, asset->randomDataByteCount);
			Utils::Stream::ClearPointer(&dest->randomDataByte);
		}

		if (asset->randomDataInt)
		{
			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->SaveArray(asset->randomDataInt, asset->randomDataIntCount);
			Utils::Stream::ClearPointer(&dest->randomDataInt);
		}

		if (asset->indices.data)
		{
			if (asset->framecount > 0xFF)
			{
				buffer->Align(Utils::Stream::ALIGN_2);
				buffer->SaveArray(asset->indices._2, asset->indexcount);
			}
			else
			{
				buffer->SaveArray(asset->indices._1, asset->indexcount);
			}

			Utils::Stream::ClearPointer(&dest->indices.data);
		}

		buffer->PopBlock();
	}
}
