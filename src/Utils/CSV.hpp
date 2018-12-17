#pragma once

namespace Utils
{
	class CSV
	{
	public:
		CSV() { }
		CSV(const std::string& file, bool isFile = true, bool allowComments = true);
		~CSV();

		int getRows();
		int getColumns();
		int getColumns(size_t row);

		std::string getElementAt(size_t row, size_t column);

		bool isValid() { return this->valid; }

	private:
		bool valid;
		std::vector<std::vector<std::string>> dataMap;

		void parse(const std::string& file, bool isFile = true, bool allowComments = true);
		void parseRow(const std::string& row, bool allowComments = true);
	};
}
