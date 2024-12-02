#include <STDInclude.hpp>
#include "AntiLag.hpp"

namespace Components
{
	Dvar::Var AntiLag::G_AntiLag;
	Dvar::Var AntiLag::G_AntiLagDebug;
	bool AntiLag::AntiLagFilled = false;

	void __cdecl AntiLag::FireWeapon(Game::gentity_s* ent, int gameTime, int eventType)
	{
		if (G_AntiLag.get<int>() == 0)
			gameTime = Game::level->time;
		else if (G_AntiLagDebug.get<int>() != 0)
			Logger::Debug("FireWeapon: at tick {}, level.time {}\n", gameTime, Game::level->time);

		return Game::FireWeapon(ent, gameTime, eventType);
	}

	void __cdecl AntiLag::FireWeaponMelee(Game::gentity_s* ent, int gameTime)
	{
		if (G_AntiLag.get<int>() == 0)
			gameTime = Game::level->time;
		else if (G_AntiLagDebug.get<int>() != 0)
			Logger::Debug("FireWeaponMelee: at tick {}, level.time {}\n", gameTime, Game::level->time);

		return Game::FireWeaponMelee(ent, gameTime);
	}

	bool AntiLag::ShouldRewind(uintptr_t stack, uintptr_t returnAddress, Game::gentity_s*& attacker)
	{
		constexpr uintptr_t Bullet_FireRet = 0x004402E0;
		constexpr uintptr_t Run_MeleeRet = 0x00501A07;
		constexpr uintptr_t G_RunMissileRet = 0x004AAD46;

		if (Bullet_FireRet == returnAddress) {
			attacker = *reinterpret_cast<Game::gentity_s**>(stack + 0x14);
			return true;
		}

		if (Run_MeleeRet == returnAddress) {
			attacker = *reinterpret_cast<Game::gentity_s**>(stack + 0x10);
			return true;
		}

		if (G_RunMissileRet != returnAddress)
		{
			// no debug check since this is totally unexpected to happen at all.
			Logger::Debug("G_AntiLagRewindClientPos called from unknown location...");
			return false;
		}

		auto projectile = *reinterpret_cast<Game::gentity_s**>(stack + 0x10);
		if (!projectile)
		{
			// no debug check since this is totally unexpected to happen at all.
			Logger::Debug("ERROR: G_AntiLagRewindClientPos called from G_RunMissile but has no projectile in stack...");
			return false;
		}

		int ownerId = projectile->r.ownerNum.number;
		if (ownerId > 0 && ownerId <= Game::level->maxclients) //
			attacker = &Game::g_entities[ownerId - 1];

		const bool hitGround = projectile->s.groundEntityNum == 0x7FE;
		auto WeaponDef = Game::BG_GetWeaponDef(projectile->s.weapon);

		// l3d note: this check below does not exist in original iw engine.
		if (hitGround && (WeaponDef->stickiness == Game::WEAPSTICKINESS_ALL
			|| WeaponDef->stickiness == Game::WEAPSTICKINESS_ALL_ORIENT
			|| WeaponDef->stickiness == Game::WEAPSTICKINESS_KNIFE))
		{
			// we did stick, no need for rewinding anymore.
			return false;
		}

		if (G_AntiLagDebug.get<int>() == 3)
		{
			// draw flying projectiles origin
			std::array<float, 4> clr = { 1.f, 1.f, 1.f, 1.f };
			std::array<float, 3> org = { 0, 0, 0 };
			Utils::Maths::VectorScale(projectile->r.box.halfSize, 3, org.data());
			Utils::Maths::VectorAdd(projectile->r.currentOrigin, org.data(), org.data());

			// draw current origin
			Game::G_DebugLineWithDuration(org.data(), projectile->r.currentOrigin, clr.data(), 1, 100);
		}

		return true;
	}

	void __cdecl AntiLag::G_AntiLagRewindClientPos(int gameTime, Game::AntilagClientStore* antilagStore)
	{
		const int antiLagMode = G_AntiLag.get<int>();
		if (antiLagMode == 0)
			return;

		Game::gentity_s* attacker = nullptr;

		// get attacker, check if we should run.
		if (!ShouldRewind(reinterpret_cast<uintptr_t>(_AddressOfReturnAddress()), reinterpret_cast<uintptr_t>(_ReturnAddress()), attacker))
			return;

		antilagStore->Reset();

		// GetClientPositionsAtTime results arrays.
		std::array<bool, 18> clientsMoved;
		std::array<Game::vec3_t, 18> clientsAngles;
		std::array<Game::vec3_t, 18> clientsPositions;

		const int deltaTime = Game::level->time - gameTime;
		const int targetTime = deltaTime > 400 ? (Game::level->time - 400) : gameTime;

		clientsMoved.fill(false);

		if (!Game::GetClientPositionAtTime(targetTime, clientsPositions.data(), clientsAngles.data(), clientsMoved.data()))
		{
			if (G_AntiLagDebug.get<int>() != 0)
				Logger::Debug("GetClientPositionAtTime failed: tick {}, level.time {}\n", targetTime, Game::level->time);

			return;
		}

		for (int i = 0; i < Game::level->maxclients; ++i)
		{
			auto& ent = Game::g_entities[i];

			bool clientMoved = clientsMoved[i];
			if (antiLagMode == 2)
			{
				// skip teammates here.
				if (attacker != nullptr && attacker->s.number != ent.s.number && Game::OnSameTeam(attacker, &ent) /*&& src_friendlyFire.get<bool>()*/)
					clientMoved = false;
			}

			if (!clientMoved)
			{
				antilagStore->clientMoved[i] = false;
				continue;
			}

			Utils::Maths::VectorCopy(ent.r.currentOrigin, antilagStore->realClientPositions[i]);
			Utils::Maths::VectorCopy(ent.r.currentAngles, antilagStore->realClientAngles[i]);

			if (G_AntiLagDebug.get<int>() == 3)
			{
				std::array<float, 4> clr = { 0.f, 1.f, 0.f, 1.f };
				std::array<float, 3> org = { ent.r.currentOrigin[0],  ent.r.currentOrigin[1],  ent.r.currentOrigin[2] + ent.r.box.halfSize[2] * 2 };

				// draw current origin
				Game::G_DebugLineWithDuration(org.data(), ent.r.currentOrigin, clr.data(), 1, 100);

				std::array<float, 4> newclr = { 1.f, 0.f, 0.f, 1.f };
				std::array<float, 3> neworg = { clientsPositions[i][0],  clientsPositions[i][1],  clientsPositions[i][2] + ent.r.box.halfSize[2] * 2 };

				// draw lag compensated origin
				Game::G_DebugLineWithDuration(neworg.data(), clientsPositions[i], newclr.data(), 1, 100);
			}

			Utils::Maths::VectorCopy(clientsAngles[i], ent.r.currentAngles);
			Utils::Maths::VectorCopy(clientsPositions[i], ent.r.currentOrigin);

			Game::SV_LinkEntity(&ent);

			if (G_AntiLagDebug.get<int>() == 2)
				Logger::Debug("antiLag: moving entity ({}) tick {}, level.time {}\n", i, targetTime, Game::level->time);

			antilagStore->clientMoved[i] = true;
		}

		AntiLag::AntiLagFilled = true;
	}

	void __cdecl AntiLag::G_AntiLag_RestoreClientPos(Game::AntilagClientStore* antilagStore)
	{
		if (!AntiLag::AntiLagFilled)
			return;

		for (int i = 0; i < Game::level->maxclients; ++i)
		{
			auto& ent = Game::g_entities[i];

			if (!antilagStore->clientMoved[i])
				continue;

			Utils::Maths::VectorCopy(antilagStore->realClientAngles[i], ent.r.currentAngles);
			Utils::Maths::VectorCopy(antilagStore->realClientPositions[i], ent.r.currentOrigin);

			Game::SV_LinkEntity(&ent);

			antilagStore->clientMoved[i] = false;
		}

		AntiLag::AntiLagFilled = false;
	}

	AntiLag::AntiLag()
	{
		G_AntiLag = Game::Dvar_RegisterInt("g_antilag", 1, 0, 2, Game::DVAR_CODINFO, "Perform antiLag.\nModes: 1 = original antiLag behaviour, 2 = optimized antiLag.");
		G_AntiLagDebug = Game::Dvar_RegisterInt("g_antilag_debug", 0, 0, 3, Game::DVAR_CHEAT, "AntiLag debugging: positions and logs.\nModes: 1 = logs, 2 = extended logs, 3 = debug overlays & extended logs.");

		AntiLag::AntiLagFilled = false;

		Utils::Hook(0x5D6D59, AntiLag::FireWeapon, HOOK_CALL).install()->quick();
		Utils::Hook(0x5D6D6C, AntiLag::FireWeaponMelee, HOOK_CALL).install()->quick();

		Utils::Hook(0x4402DB, AntiLag::G_AntiLagRewindClientPos, HOOK_CALL).install()->quick(); // Bullet_Fire
		Utils::Hook(0x4404EA, AntiLag::G_AntiLag_RestoreClientPos, HOOK_CALL).install()->quick(); // Bullet_Fire

		Utils::Hook(0x4AAD41, AntiLag::G_AntiLagRewindClientPos, HOOK_CALL).install()->quick(); // Run_Missile
		Utils::Hook(0x4AAD5C, AntiLag::G_AntiLag_RestoreClientPos, HOOK_CALL).install()->quick(); // Run_Missile

		Utils::Hook(0x501A02, AntiLag::G_AntiLagRewindClientPos, HOOK_CALL).install()->quick(); // Run_Melee (name was guessed)
		Utils::Hook(0x501A42, AntiLag::G_AntiLag_RestoreClientPos, HOOK_CALL).install()->quick(); // Run_Melee (name was guessed)
	}
}
