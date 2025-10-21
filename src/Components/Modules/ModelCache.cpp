#include "ModelCache.hpp"

namespace Components
{
	Game::XModel* ModelCache::cached_models_reallocated[G_MODELINDEX_LIMIT];
	Game::XModel* ModelCache::gameModels_reallocated[G_MODELINDEX_LIMIT]; // Partt of cgs_t
	bool ModelCache::modelsHaveBeenReallocated;

	void ModelCache::R_RegisterModel_InitGraphics(const char* name, void* atAddress)
	{
		const unsigned int gameModelsOriginalAddress = 0x7ED658; // Don't put in Game::
		unsigned int index = (reinterpret_cast<unsigned int>(atAddress) - gameModelsOriginalAddress) / sizeof(Game::XModel*);
		index++; // Models start at 1 (index 0 is unused)

		// R_REgisterModel
		gameModels_reallocated[index] = Utils::Hook::Call<Game::XModel * (const char*)>(0x50FA00)(name);
	}

	__declspec(naked) void ModelCache::R_RegisterModel_Hook()
	{
		_asm
		{
			pushad;
			push esi;
			push eax;
			call R_RegisterModel_InitGraphics;
			pop esi;
			pop eax;
			popad;

			push 0x58991B;
			retn;
		}
	}

	ModelCache::ModelCache()
	{
		static constexpr std::array<std::string_view, 9> fieldsToUpdate
		{
			// clientState
			"modelindex",
			"attachModelIndex[0]",
			"attachModelIndex[1]",
			"attachModelIndex[2]",
			"attachModelIndex[3]",
			"attachModelIndex[4]",
			"attachModelIndex[5]",
		
			// playerState
			"viewmodelIndex",
		
			// entityState
			"lerp.u.scriptMover.xModelIndex"
		};
		
		// To push the model limit we need to update the network protocol because it uses custom integer size
		// (currently 9 bits per model, not enough)
		static constexpr auto oldBitLength = static_cast<size_t>(std::bit_width(static_cast<size_t>(BASE_GMODEL_COUNT - 1)));
		static constexpr auto newBitLength = static_cast<size_t>(std::bit_width(static_cast<size_t>(G_MODELINDEX_LIMIT - 1)));
		static_assert(oldBitLength == 9 && newBitLength == 12);
		
		std::array<bool, fieldsToUpdate.size()> found{};
		
		for (auto& netfield : Game::netfields)
		{
			for (size_t i = 0; i < fieldsToUpdate.size(); ++i)
			{
				if (!found[i] && netfield.name == fieldsToUpdate[i])
				{
					assert(static_cast<size_t>(netfield.bits) == oldBitLength);
		
					Utils::Hook::Set(&netfield.bits, newBitLength);
					found[i] = true;
		
					assert(static_cast<size_t>(netfield.bits) == newBitLength);
					break;
				}
			}
		}
		
		assert(std::accumulate(found.begin(), found.end(), 0u) == found.size());

		for (auto& netfield : Game::netfields)
		{
			static_assert(offsetof(Game::entityState_s, index) == 144);
		
			if (netfield.offset == offsetof(Game::entityState_s, index) && (netfield.name == "index"sv || netfield.name == "index.item"sv))
			{
				// Some of the bit lengths for these index fields are already 15
				assert(netfield.bits <= 15);
				
				Utils::Hook::Set(&netfield.bits, std::max(15u, newBitLength));
			}
		}
		
		// Reallocate G_ModelIndex
		Utils::Hook::Set(0x420654 + 3, ModelCache::cached_models_reallocated);
		Utils::Hook::Set(0x43BCE4 + 3, ModelCache::cached_models_reallocated);
		Utils::Hook::Set(0x44F27B + 3, ModelCache::cached_models_reallocated);
		Utils::Hook::Set(0x479087 + 1, ModelCache::cached_models_reallocated);
		Utils::Hook::Set(0x48069D + 3, ModelCache::cached_models_reallocated);
		Utils::Hook::Set(0x48F088 + 3, ModelCache::cached_models_reallocated);
		Utils::Hook::Set(0x4F457C + 3, ModelCache::cached_models_reallocated);
		Utils::Hook::Set(0x5FC762 + 3, ModelCache::cached_models_reallocated);
		Utils::Hook::Set(0x5FC7BE + 3, ModelCache::cached_models_reallocated);

		// Reallocate cg models
		Utils::Hook::Set(0x44506D + 3, ModelCache::gameModels_reallocated);
		Utils::Hook::Set(0x46D49C + 3, ModelCache::gameModels_reallocated);
		Utils::Hook::Set(0x586015 + 3, ModelCache::gameModels_reallocated);
		Utils::Hook::Set(0x586613 + 3, ModelCache::gameModels_reallocated);
		Utils::Hook::Set(0x594E1D + 3, ModelCache::gameModels_reallocated);

		// This one is offset by a compiler optimization
		Utils::Hook::Set(0x592B3D + 3, reinterpret_cast<int>(ModelCache::gameModels_reallocated) - Game::CS_MODELS * sizeof(Game::XModel*));

		// This one is annoying to catch
		Utils::Hook(0x589916, R_RegisterModel_Hook, HOOK_JUMP).install()->quick();

		// Patch limit
		Utils::Hook::Set(0x479080 + 1, ModelCache::G_MODELINDEX_LIMIT * sizeof(void*));
		Utils::Hook::Set<DWORD>(0x44F256 + 2, ModelCache::G_MODELINDEX_LIMIT);
		Utils::Hook::Set<DWORD>(0x44F231 + 2, ModelCache::G_MODELINDEX_LIMIT);

		modelsHaveBeenReallocated = true;
	}
}
