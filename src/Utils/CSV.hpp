#pragma once

namespace Utils
{
	class CSV
	{
	public:
		CSV(const std::string& file, bool isFile = true, bool allowComments = true);

		[[nodiscard]] std::size_t getRows() const;
		[[nodiscard]] std::size_t getColumns() const;
		[[nodiscard]] std::size_t getColumns(std::size_t row) const;

		[[nodiscard]] std::string getElementAt(std::size_t row, std::size_t column) const;

		[[nodiscard]] bool isValid() const;

	private:
		bool valid_ = false;
		std::vector<std::vector<std::string>> dataMap_;

		void parse(const std::string& file, bool isFile = true, bool allowComments = true);
		void parseRow(const std::string& row, bool allowComments = true);
	};
}
