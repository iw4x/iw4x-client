namespace Components
{
	class Weapon : public Component
	{
	public:
		Weapon();
		const char* GetName() { return "Weapon"; };

	private:
		static Game::XAssetHeader WeaponFileLoad(Game::XAssetType type, const char* filename);
	};
}
