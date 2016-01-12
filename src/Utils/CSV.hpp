namespace Utils
{
	class CSV
	{
	public:
		CSV(std::string file, bool isFile = true);
		~CSV();

		int GetRows();
		int GetColumns();
		int GetColumns(size_t row);

		std::string GetElementAt(size_t row, size_t column);

	private:

		void Parse(std::string file, bool isFile = true);
		void ParseRow(std::string row);
		std::vector<std::vector<std::string>> DataMap;
	};
}
