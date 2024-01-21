class IWeaponCompleteDef_461
{
	#define WEAPON_ASSET_SIZE 4124

	struct WeaponCompleteDef
	{
		char* szInternalName; // +0
		char* szDisplayName;		// +4
		char* szInternalName2;		// +8
		char* unkString3;		// +12
		Game::XModel* unkModels1[2];	// +16
		Game::XModel* unkModels2[2];	// +24
		Game::XModel* unkModels_Multiple1[6];			// +32
		char unkPadding[4];
		int16_t unkScriptStrings[32];		// +60
		char** unkXStringArray1;		// +124
		char unkPadding3[244];
		char** unkXStringArray2;		// +372
		char unkPadding4[244];
		char** unkXStringArray3;		// +620

		char unkPadding5[276];			// + 624

		int16_t unkScriptString; // 900
		char unkPadding5a[30];

		int16_t unkScriptString2; // 932
		char unkPadding5b[30];

		int16_t unkScriptString3; // 964
		char unkPadding5c[30];

		int16_t unkScriptString4; // 996
		char unkPadding5d[30];

		Game::FxEffectDef* effects[16]; // +1028
		int16_t* unkScriptString5; // +1092
		char unkPadding6[28];

		Game::FxEffectDef* effects2[16]; // +1124
		Game::FxEffectDef* effects2b[16]; // + 1188
		char unkPadding7[64]; // + 1188

		Game::FxEffectDef* effects3[16]; // +1316
		char unkPadding8[64]; // + 1380

		Game::FxEffectDef* viewFlashEffect; // +1444
		Game::FxEffectDef* worldFlashEffect; // +1448
		Game::FxEffectDef* unkEffect3; // +1452
		Game::FxEffectDef* unkEffect4; // +1456
		Game::FxEffectDef* unkEffect5; // +1460

		char* unkString4;				// +1464
		int unkPadding9;

		char* unkString5;				// +1472
		Game::snd_alias_list_t* sounds[53]; // +1476
		Game::snd_alias_list_t* sounds1b[5]; // +1604

		//Game::snd_alias_list_t** soundsList[5]; // 1688
		Game::snd_alias_list_t** bounceSounds1; // + 1708
		Game::snd_alias_list_t** bounceSounds2; // + 1712
		Game::FxEffectDef* viewShellEjectEffect;	// 1716
		Game::FxEffectDef* worldShellEjectEffect;
		Game::FxEffectDef* viewLastShotEjectEffect;
		Game::FxEffectDef* worldLastShotEjectEffect;
		Game::Material* reticleCenter;		// 1732
		Game::Material* reticleSide;		// 1736
		char unkPadding10[212];

		Game::Material* materials[6]; // 1952
		int unkPadding11;

		Game::Material* material3; // 1980
		int unkPadding12;

		Game::Material* material4; // 1988
		char unkPadding13[8];

		Game::Material* overlayMaterial; // 2000
		Game::Material* overlayMaterialLowRes;
		Game::Material* overlayMaterialEMP;
		Game::Material* overlayMaterialEMPLowRes;

		char unkPadding14[8];

		char* unkString6; // 2024
		int unkPadding15;

		char* unkString7; // 2032
		char unkPadding16[12];

		char* unkString8; // 2048

		char padFloats[278];

		Game::Material* unkMaterials2[4]; // 2332

		char padFloats2[194];

		Game::PhysCollmap* physCollMap; // 2544;

		Game::PhysPreset* physPreset; // 2548

		char padFloats3[104];

		Game::XModel* projectileModel; // 2656
		Game::weapProjExposion_t projExplosion; // 2660
		Game::FxEffectDef* projExplosionEffect; // 2664
		Game::FxEffectDef* projDudEffect; // 2668
		Game::snd_alias_list_t* projDudSound; // 2672
		Game::snd_alias_list_t* unkSound1; // 2676

		char padProj2[272];

		Game::FxEffectDef* unkProjEffects1[2]; // 2952

		char padFX[24];

		Game::FxEffectDef* unkEffect5b;

		Game::snd_alias_list_t* unkSound2; // 2988
		Game::FxEffectDef* projTrailEffect;  // 2992
		Game::FxEffectDef* projBeaconEffect; // 2996
		Game::FxEffectDef* unkProjEffects2[3]; // 3000

		char padProj3[184];

		char* accuracyGraphName1; // 3196
		char* accuracyGraphName2; // 3200
		 // OUT OF ORDER!
		uint64_t* unknownAsset1; // 3204
		uint64_t* unknownAsset2; // 3208
		char unkPadding16_b[80];

		char* unkString10_b; // 3292
		char* unkString11; // 3296

		char padStrings[28];	

		char* unkString12; // 3328
		char* unkString13; // 3332
		char padStrings2[152];

		char* unkStrings[2]; // 3488

		Game::TracerDef* tracers[3];

		char padStrings3[24];

		Game::snd_alias_list_t* unkSound3; // 3532
		Game::FxEffectDef* unkEffect6; // 3536
		char* unkString14; // 3540

		char unkPadding17[12];

		Game::snd_alias_list_t* sounds2; // 3556
		Game::snd_alias_list_t* turretBarrelSpinUpSnd[4]; // 3560
		Game::snd_alias_list_t* turretBarrelSpinDownSnd[4]; // 3576
		Game::snd_alias_list_t* sounds3[2]; // 3592

		char unkPadding18[64];

		Game::snd_alias_list_t* sounds4[6]; // 3664

		char unkPadding19[26];

		char* unkString15; // 3716
		char unkPadding20[12];

		char* unkString16; // 3732
		char* unkString17; // 3736
		char unkPadding21[4];

		Game::Material* unkMaterials3[4]; // 3744
		char unkPadding22[20];
		unsigned short numberOfUnknownAsset1;
		unsigned short numberOfUnknownAsset2;

		uint64_t* unknownAsset3; // 3784
		uint64_t* unknownAsset4; // 3788
		char unkPadding23[88];

		char* unkString18[3]; // 3880
		char unkPadding24[108];

		char* unkString19; // 4000
		char unkPadding25[12];

		char* unkString20; // 4016

		char padEnd[104];
	};

	static_assert(offsetof(WeaponCompleteDef, unkXStringArray1) == 124);
	static_assert(offsetof(WeaponCompleteDef, unkXStringArray2) == 372);
	static_assert(offsetof(WeaponCompleteDef, unkXStringArray3) == 620);
	static_assert(offsetof(WeaponCompleteDef, effects) == 1028);
	static_assert(offsetof(WeaponCompleteDef, effects2) == 1124);
	static_assert(offsetof(WeaponCompleteDef, effects3) == 1316);
	static_assert(offsetof(WeaponCompleteDef, viewFlashEffect) == 1444);
	static_assert(offsetof(WeaponCompleteDef, unkEffect5) == 1460);
	static_assert(offsetof(WeaponCompleteDef, sounds) == 1476);
	static_assert(offsetof(WeaponCompleteDef, viewShellEjectEffect) == 1716);
	static_assert(offsetof(WeaponCompleteDef, reticleSide) == 1736);
	static_assert(offsetof(WeaponCompleteDef, materials) == 1952);
	static_assert(offsetof(WeaponCompleteDef, material3) == 1980);
	static_assert(offsetof(WeaponCompleteDef, material4) == 1988);
	static_assert(offsetof(WeaponCompleteDef, overlayMaterial) == 2000);
	static_assert(offsetof(WeaponCompleteDef, unkString6) == 2024);
	static_assert(offsetof(WeaponCompleteDef, unkString8) == 2048);
	static_assert(offsetof(WeaponCompleteDef, unkMaterials2) == 2332);
	static_assert(offsetof(WeaponCompleteDef, physCollMap) == 2544);
	static_assert(offsetof(WeaponCompleteDef, projectileModel) == 2656);
	static_assert(offsetof(WeaponCompleteDef, unkSound1) == 2676);
	static_assert(offsetof(WeaponCompleteDef, unkProjEffects1) == 2952);
	static_assert(offsetof(WeaponCompleteDef, unkProjEffects2) == 3000);
	static_assert(offsetof(WeaponCompleteDef, accuracyGraphName1) == 3196);
	static_assert(offsetof(WeaponCompleteDef, unknownAsset1) == 3204);
	static_assert(offsetof(WeaponCompleteDef, unkString10_b) == 3292);
	static_assert(offsetof(WeaponCompleteDef, unkString11) == 3296);
	static_assert(offsetof(WeaponCompleteDef, unkString12) == 3328);
	static_assert(offsetof(WeaponCompleteDef, unkStrings) == 3488);
	static_assert(offsetof(WeaponCompleteDef, unkString14) == 3540);
	static_assert(offsetof(WeaponCompleteDef, sounds2) == 3556);
	static_assert(offsetof(WeaponCompleteDef, turretBarrelSpinUpSnd) == 3560);
	static_assert(offsetof(WeaponCompleteDef, turretBarrelSpinDownSnd) == 3576);
	static_assert(offsetof(WeaponCompleteDef, sounds3) == 3592);
	static_assert(offsetof(WeaponCompleteDef, sounds4) == 3664);
	static_assert(offsetof(WeaponCompleteDef, unkString15) == 3716);
	static_assert(offsetof(WeaponCompleteDef, unkString16) == 3732);
	static_assert(offsetof(WeaponCompleteDef, unkMaterials3) == 3744);
	static_assert(offsetof(WeaponCompleteDef, unknownAsset3) == 3784);
	static_assert(offsetof(WeaponCompleteDef, unknownAsset4) == 3788);
	static_assert(offsetof(WeaponCompleteDef, unkString18) == 3880);
	static_assert(offsetof(WeaponCompleteDef, unkString19) == 4000);
	static_assert(offsetof(WeaponCompleteDef, unkString20) == 4016);

	static_assert(sizeof(IWeaponCompleteDef_461::WeaponCompleteDef) >= WEAPON_ASSET_SIZE, "Invalid size");
	static_assert(sizeof(IWeaponCompleteDef_461::WeaponCompleteDef) <= WEAPON_ASSET_SIZE, "Invalid size");
	static_assert(sizeof(IWeaponCompleteDef_461::WeaponCompleteDef) == WEAPON_ASSET_SIZE, "Invalid size");

	public:
		static void LoadWeaponCompleteDef()
		{
			const auto def = Read461WeaponCompleteDef();
			Convert461WeaponCompleteDefTo276(def);
		};

	private:
		static WeaponCompleteDef Read461WeaponCompleteDef();
		static void Convert461WeaponCompleteDefTo276(const WeaponCompleteDef& definition);

};
