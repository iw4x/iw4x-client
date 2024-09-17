#pragma once

namespace Components
{
	class MapRotation : public Component
	{
	public:
		MapRotation();

		static bool Contains(const std::string& key, const std::string& value);

		static nlohmann::json to_json();

		bool unitTest() override;

	private:
		class MapRotationParseError : public std::runtime_error
		{
		private:
			static std::string fmt(const std::string& message)
			{
				std::string error = "Map Rotation Parse Error";

				if (!message.empty())
				{
					error.append(": ");
					error.append(message);
				}

				return error;
			}

		public:
			MapRotationParseError(const std::string& message)
				: std::runtime_error(fmt(message))
			{
			}
		};

		class RotationData
		{
		public:
			using rotationEntry = std::pair<std::string, std::string>;

			using rotationCallback = std::function<void(const std::string&)>;

			RotationData();

			void randomize();

			// In case a new way to enrich the map rotation is added (other than sv_mapRotation)
			// this method should be called to add a new entry (gamemode/map & value)
			void addEntry(const std::string& key, const std::string& value);

			[[nodiscard]] std::size_t getEntriesSize() const noexcept;
			rotationEntry& getNextEntry();
			rotationEntry& peekNextEntry();

			void setHandler(const std::string& key, const rotationCallback& callback);
			void callHandler(const rotationEntry& entry) const;

			void parse(const std::string& data);

			[[nodiscard]] bool empty() const noexcept;
			[[nodiscard]] bool contains(const std::string& key, const std::string& value) const;
			[[nodiscard]] bool containsHandler(const std::string& key) const;

			void clear() noexcept;

			[[nodiscard]] nlohmann::json to_json() const;

		private:
			std::vector<rotationEntry> rotationEntries_;
			std::unordered_map<std::string, rotationCallback> rotationHandlers_;

			std::size_t index_;
		};

		// Rotation Dvars
		static Dvar::Var SVRandomMapRotation;
		static Dvar::Var SVDontRotate;
		static Dvar::Var SVNextMap;

		// Holds the parsed data from sv_mapRotation
		static RotationData DedicatedRotation;

		static void RandomizeMapRotation();
		static void ParseRotation(const std::string& data);
		static void LoadMapRotation();

		// Use these commands before SV_MapRotate_f is called
		static void AddMapRotationCommands();
		static void RegisterMapRotationDvars();

		static bool ShouldRotate();
		static void ApplyMap(const std::string& map);
		static void ApplyGametype(const std::string& gametype);
		static void ApplyExec(const std::string& name);
		static void RestartCurrentMap();
		static void ApplyRotation(RotationData& rotation);
		static void ApplyMapRotationCurrent(const std::string& data);

		// Utils functions
		static void SetNextMap(RotationData& rotation); // Only call this after ApplyRotation
		static void SetNextMap(const char* value);
		static void ClearNextMap();

		static void SV_MapRotate_f();
	};
}
