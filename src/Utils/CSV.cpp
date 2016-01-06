#include "STDInclude.hpp"

namespace Utils
{
	CSV::CSV(std::string file)
	{
		CSV::Parse(file);
	}

	CSV::~CSV()
	{
		for (auto row : CSV::DataMap)
		{
			for (auto entry : row)
			{
				entry.clear();
			}

			row.clear();
		}

		CSV::DataMap.clear();
	}

	int CSV::GetRows()
	{
		return CSV::DataMap.size();
	}

	int CSV::GetColumns(size_t row)
	{
		if (CSV::DataMap.size() > row)
		{
			return CSV::DataMap[row].size();
		}

		return 0;
	}

	std::string CSV::GetElementAt(size_t row, size_t column)
	{
		if (CSV::DataMap.size() > row)
		{
			auto _row = CSV::DataMap[row];

			if (_row.size() > column)
			{
				return _row[column];
			}
		}

		return "";
	}

	void CSV::Parse(std::string file)
	{
		if (!Utils::FileExists(file)) return;

		std::string buffer = Utils::ReadFile(file);
		
		if (buffer.size())
		{
			auto rows = Utils::Explode(buffer, '\n');

			for (auto row : rows)
			{
				CSV::ParseRow(row);
			}
		}
	}

	void CSV::ParseRow(std::string row)
	{
		bool isString = false;
		std::string element;
		std::vector<std::string> _row;
		char tempStr[2] = { 0, 0 };

		for (unsigned int i = 0; i < row.size(); i++)
		{
			if (row[i] == ',' && !isString) // FLush entry
			{
				_row.push_back(element);
				element.clear();
				continue;
			}
			else if (row[i] == '"') // Start/Terminate string
			{
				isString = !isString;
				continue;
			}
			else if (i < (row.size() - 1) && row[i] == '\\' &&row[i + 1] == '"' && isString) // Handle quotes in strings as \"
			{
				tempStr[0] = '"';
				++i;
			}
			else if (!isString && (row[i] == '\n' || row[i] == '\x0D' || row[i] == '\x0A' || row[i] == '\t'))
			{
				//++i;
				continue;
			}
			else if (!isString && row[i] == '#') // Skip comments. I know CSVs usually don't have comments, but in this case it's useful
			{
				return;
			}
			else
			{
				tempStr[0] = row[i];
			}

			element.append(tempStr);
		}

		// Push last element
		_row.push_back(element);

		if (_row.size() == 0 || (_row.size() == 1 && !_row[0].size())) // Skip empty rows
		{
			return;
		}

		DataMap.push_back(_row);
	}
}
