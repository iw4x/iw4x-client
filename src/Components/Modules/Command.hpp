#pragma once

namespace Components
{
	class Command : public Component
	{
	public:
		class Params
		{
		public:
			Params() {};
			virtual ~Params() {};
			virtual const char* get(size_t index) = 0;
			virtual size_t length() = 0;

			virtual std::string join(size_t startIndex);
			virtual const char* operator[](size_t index);
		};

		class ClientParams : public Params
		{
		public:
			ClientParams(unsigned int id) : commandId(id) {};
			ClientParams(const ClientParams &obj) : commandId(obj.commandId) {};
			ClientParams() : ClientParams(*Game::cmd_id) {};

			const char* get(size_t index) override;
			size_t length() override;

		private:
			unsigned int commandId;
		};

		class ServerParams : public Params
		{
		public:
			ServerParams(unsigned int id) : commandId(id) {};
			ServerParams(const ServerParams &obj) : commandId(obj.commandId) {};
			ServerParams() : ServerParams(*Game::cmd_id_sv) {};

			const char* get(size_t index) override;
			size_t length() override;

		private:
			unsigned int commandId;
		};

		typedef void(Callback)(Command::Params* params);

		Command();
		~Command();

		static Game::cmd_function_t* Allocate();

		static void Add(const char* name, Utils::Slot<Callback> callback);
		static void AddSV(const char* name, Utils::Slot<Callback> callback);
		static void AddRaw(const char* name, void(*callback)(), bool key = false);
		static void AddRawSV(const char* name, void(*callback)());
		static void Execute(std::string command, bool sync = true);

		static Game::cmd_function_t* Find(const std::string& command);

	private:
		static std::unordered_map<std::string, Utils::Slot<Callback>> FunctionMap;
		static std::unordered_map<std::string, Utils::Slot<Callback>> FunctionMapSV;

		static void MainCallback();
		static void MainCallbackSV();
	};
}
