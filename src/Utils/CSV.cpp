#include <STDInclude.hpp>

namespace Utils
{
	CSV::CSV(const std::string& file, const bool isFile, const bool allowComments)
	{
		this->parse(file, isFile, allowComments);
	}

	std::size_t CSV::getRows() const
	{
		return this->dataMap_.size();
	}

	std::size_t CSV::getColumns() const
	{
		std::size_t count = 0;

		for (std::size_t i = 0; i < this->getRows(); ++i)
		{
			count = std::max(this->getColumns(i), count);
		}

		return count;
	}

	std::size_t CSV::getColumns(const std::size_t row) const
	{
		if (this->dataMap_.size() > row)
		{
			return this->dataMap_[row].size();
		}

		return 0;
	}

	std::string CSV::getElementAt(const std::size_t row, const std::size_t column) const
	{
		if (this->dataMap_.size() > row)
		{
			auto& _row = this->dataMap_[row];

			if (_row.size() > column)
			{
				return _row[column];
			}
		}

		return {};
	}

	bool CSV::isValid() const
	{
		return this->valid_;
	}

	void CSV::parse(const std::string& file, const bool isFile, const bool allowComments)
	{
		std::string buffer;

		if (isFile)
		{
			if (!IO::FileExists(file))
			{
				return;
			}

			buffer = IO::ReadFile(file);
			this->valid_ = true;
		}
		else
		{
			buffer = file;
		}

		if (!buffer.empty())
		{
			const auto rows = String::Split(buffer, '\n');

			for (auto& row : rows)
			{
				this->parseRow(row, allowComments);
			}
		}
	}

	void CSV::parseRow(const std::string& row, const bool allowComments)
	{
		bool isString = false;
		std::string element;
		std::vector<std::string> _row;
		char tempStr = 0;

		for (std::size_t i = 0; i < row.size(); ++i)
		{
			if (row[i] == ',' && !isString) // Flush entry
			{
				_row.push_back(element);
				element.clear();
				continue;
			}

			if (row[i] == '"') // Start/Terminate string
			{
				isString = !isString;
				continue;
			}

			if (i < (row.size() - 1) && row[i] == '\\' &&row[i + 1] == '"' && isString) // Handle quotes in strings as \"
			{
				tempStr = '"';
				++i;
			}

			else if (!isString && (row[i] == '\n' || row[i] == '\x0D' || row[i] == '\x0A' || row[i] == '\t'))
			{
				continue;
			}

			else if (!isString && (row[i] == '#' || (row[i] == '/' && (i + 1) < row.size() && row[i + 1] == '/') ) && allowComments) // Skip comments. I know CSVs usually don't have comments, but in this case it's useful
			{
				return;
			}

			else
			{
				tempStr = row[i];
			}

			element.append(&tempStr, 1);
		}

		// Push last element
		_row.push_back(element);

		if (_row.empty() || (_row.size() == 1 && _row[0].empty())) // Skip empty rows
		{
			return;
		}

		this->dataMap_.push_back(_row);
	}
}
