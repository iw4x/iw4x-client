#pragma once

namespace Components
{
	class Command : public Component
	{
	public:
		static_assert(sizeof(Game::cmd_function_s) == 0x18);

		class Params
		{
		public:
			Params() = default;
			virtual ~Params() = default;

			Params(Params&&) = delete;
			Params(const Params&) = delete;
			Params& operator=(Params&&) = delete;
			Params& operator=(const Params&) = delete;

			[[nodiscard]] virtual int size() const noexcept = 0;
			[[nodiscard]] virtual const char* get(int index) const noexcept = 0;
			[[nodiscard]] virtual std::string join(int index) const;

			virtual const char* operator[](const int index)
			{
				return this->get(index);
			}
		};

		class ClientParams final : public Params
		{
		public:
			ClientParams();

			[[nodiscard]] int size() const noexcept override;
			[[nodiscard]] const char* get(int index) const noexcept override;

		private:
			int nesting_;
		};

		class ServerParams final : public Params
		{
		public:
			ServerParams();

			[[nodiscard]] int size() const noexcept override;
			[[nodiscard]] const char* get(int index) const noexcept override;

		private:
			int nesting_;
		};

		Command();

		using commandCallback = std::function<void(const Params*)>;

		static void Add(const char* name, const std::function<void()>& callback);
		static void Add(const char* name, const commandCallback& callback);
		static void AddRaw(const char* name, void(*callback)(), bool key = false);
		static void AddSV(const char* name, const commandCallback& callback);
		static void Execute(std::string command, bool sync = true);

		static Game::cmd_function_s* Find(const std::string& command);

	private:
		static std::unordered_map<std::string, commandCallback> FunctionMap;
		static std::unordered_map<std::string, commandCallback> FunctionMapSV;

		static Game::cmd_function_s* Allocate();

		static void AddRawSV(const char* name, void(*callback)());

		static void MainCallback();
		static void MainCallbackSV();

		static const std::vector<std::string>& GetExceptions();
		static bool CL_ShouldSendNotify_Hk(const char* cmd);
	};
}
