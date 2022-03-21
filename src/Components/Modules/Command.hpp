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

			virtual int size() = 0;
			virtual const char* get(int index) = 0;
			virtual std::string join(int index);

			virtual const char* operator[](const int index)
			{
				return this->get(index);
			}
		};

		class ClientParams : public Params
		{
		public:
			ClientParams();

			int size() override;
			const char* get(int index) override;

		private:
			int nesting_;
		};

		class ServerParams : public Params
		{
		public:
			ServerParams();

			int size() override;
			const char* get(int index) override;

		private:
			int nesting_;
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
