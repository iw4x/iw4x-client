namespace Components
{
	class FastFiles : public Component
	{
	public:

		class Offset
		{
		public:
			union
			{
				struct
				{
					uint32_t pointer : 28;
					int stream : 4;
				};
				uint32_t fullValue;
				void* fullPointer;
			};

			uint32_t GetDecrementedPointer()
			{
				Offset offset = *this;
				offset.fullValue--;
				return offset.pointer;
			};

			int GetDecrementedStream()
			{
				Offset offset = *this;
				offset.fullValue--;
				return offset.stream;
			};
		};

		FastFiles();
		~FastFiles();
		const char* GetName() { return "FastFiles"; };

		static void AddZonePath(std::string path);
		static std::string Current();

	private:
		static std::vector<std::string> ZonePaths;
		static const char* GetZoneLocation(const char* file);
		static void LoadDLCUIZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
	};
}
