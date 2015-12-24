namespace Components
{
	class FileSystem : public Component
	{
	public:

		class File
		{
		public:
			//File() {};
			File(std::string file) : FilePath(file) { this->Read(); };

			bool Exists() { return this->Buffer.size() > 0; };
			std::string GetName() { return this->FilePath; };
			std::string& GetBuffer() { return this->Buffer; };

		private:
			std::string FilePath;
			std::string Buffer;

			void Read();
		};

		FileSystem();
		const char* GetName() { return "FileSystem"; };
	};
}
