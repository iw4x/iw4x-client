namespace Components
{
	class Command : public Component
	{
	public:
		class IParams
		{
		public:
			IParams() {};
			virtual ~IParams() {};
			virtual char* operator[](size_t index) = 0;
			virtual size_t length() = 0;
			virtual std::string join(size_t startIndex);
		};

		class IClientParams : public IParams
		{
		public:
			IClientParams(unsigned int id) : commandId(id) {};
			IClientParams(const IClientParams &obj) : commandId(obj.commandId) {};
			IClientParams() : IClientParams(*Game::cmd_id) {};

			~IClientParams() {};
			char* operator[](size_t index) override;
			size_t length() override;

		private:
			unsigned int commandId;
		};

		class IServerParams : public IParams
		{
		public:
			IServerParams(unsigned int id) : commandId(id) {};
			IServerParams(const IServerParams &obj) : commandId(obj.commandId) {};
			IServerParams() : IServerParams(*Game::cmd_id_sv) {};

			~IServerParams() {};
			char* operator[](size_t index) override;
			size_t length() override;

		private:
			unsigned int commandId;
		};

		class Params
		{
		public:
			Params(IParams* _paramInterface) : paramInterface(_paramInterface) 
			{ 
				if (!paramInterface) 
				{
					throw new std::invalid_argument("Invalid command parameter interface!");
				}
			};

			Params(const Params &obj) : paramInterface(obj.paramInterface) {};

			char* operator[](size_t index) 
			{ 
				return paramInterface->operator[](index);
			}

			size_t length()
			{
				return paramInterface->length();
			}

			std::string join(size_t startIndex) 
			{ 
				return paramInterface->join(startIndex);
			}

		private:
			IParams* paramInterface;
		};

		typedef void(Callback)(Command::Params params);

		Command();
		~Command();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "Command"; };
#endif

		static Game::cmd_function_t* Allocate();

		static void Add(const char* name, Callback* callback);
		static void AddSV(const char* name, Callback* callback);
		static void AddRaw(const char* name, void(*callback)(), bool key = false);
		static void AddRawSV(const char* name, void(*callback)());
		static void Execute(std::string command, bool sync = true);

		static Game::cmd_function_t* Find(std::string command);

	private:
		static Utils::Memory::Allocator MemAllocator;
		static std::map<std::string, wink::slot<Callback>> FunctionMap;
		static std::map<std::string, wink::slot<Callback>> FunctionMapSV;

		static void MainCallback();
		static void MainCallbackSV();
	};
}
