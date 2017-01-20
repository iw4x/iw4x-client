#pragma once

namespace Utils
{
	class CSV
	{
	public:
		CSV(std::string file, bool isFile = true, bool allowComments = true);
		~CSV();

		int getRows();
		int getColumns();
		int getColumns(size_t row);

		std::string getElementAt(size_t row, size_t column);

	private:
		std::vector<std::vector<std::string>> dataMap;

		void parse(std::string file, bool isFile = true, bool allowComments = true);
		void parseRow(std::string row, bool allowComments = true);
	};
}
