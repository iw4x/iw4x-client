namespace Utils
{
	class CSV
	{
	public:
		CSV(std::string file);
		~CSV();

		int GetRows();
		int GetColumns(size_t row);

		std::string GetElementAt(size_t row, size_t column);

	private:

		void Parse(std::string file);
		void ParseRow(std::string row);
		std::vector<std::vector<std::string>> DataMap;
	};
}
