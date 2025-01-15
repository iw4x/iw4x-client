
namespace Utils
{
	std::string Entities::build() const
	{
		std::string entityString;

		for (const auto& entity : this->entities)
		{
			entityString.append("{\n");

			for (const auto& property : entity)
			{
				entityString.append("\"");
				entityString.append(property.first);
				entityString.append("\" \"");
				entityString.append(property.second);
				entityString.append("\"\n");
			}

			entityString.append("}\n");
		}

		return entityString;
	}

	std::vector<std::string> Entities::getModels()
	{
		std::vector<std::string> models;

		for (auto& entity : this->entities)
		{
			if (const auto itr = entity.find("model"); itr != entity.end())
			{
				const auto& model = itr->second;

				if (!model.empty() && model[0] != '*' && model[0] != '?' &&  // Skip brushmodels
					model != "com_plasticcase_green_big_us_dirt"s // Team zones
				)
				{
					if (std::find(models.begin(), models.end(), model) == models.end())
					{
						models.push_back(model);
					}
				}
			}
		}

		return models;
	}

	std::vector<std::string> Entities::getWeapons()
	{
		std::vector<std::string> weapons;

		for (auto& entity : this->entities)
		{
			if (const auto itr = entity.find("weaponinfo"); itr != entity.end())
			{
				const auto& weapon = itr->second;

				if (!weapon.empty())
				{
					if (std::find(weapons.begin(), weapons.end(), weapon) == weapons.end())
					{
						weapons.push_back(weapon);
					}
				}
			}
		}

		return weapons;
	}

	void Entities::parse(const std::string& buffer)
	{
		int parseState = 0;
		std::string key;
		std::string value;
		std::unordered_map<std::string, std::string> entity;

		for (std::size_t i = 0; i < buffer.size(); ++i)
		{
			const auto character = buffer[i];
			if (character == '{')
			{
				entity.clear();
			}

			switch (character)
			{
			case '{':
			{
				entity.clear();
				break;
			}

			case '}':
			{
				this->entities.push_back(entity);
				entity.clear();
				break;
			}

			case '"':
			{
				if (parseState == PARSE_AWAIT_KEY)
				{
					key.clear();
					parseState = PARSE_READ_KEY;
				}
				else if (parseState == PARSE_READ_KEY)
				{
					parseState = PARSE_AWAIT_VALUE;
				}
				else if (parseState == PARSE_AWAIT_VALUE)
				{
					value.clear();
					parseState = PARSE_READ_VALUE;
				}
				else if (parseState == PARSE_READ_VALUE)
				{
					entity[String::ToLower(key)] = value;
					parseState = PARSE_AWAIT_KEY;
				}
				else
				{
					throw std::runtime_error("Parsing error!");
				}
				break;
			}

			default:
			{
				if (parseState == PARSE_READ_KEY) key.push_back(character);
				else if (parseState == PARSE_READ_VALUE) value.push_back(character);

				break;
			}
			}
		}
	}
}
