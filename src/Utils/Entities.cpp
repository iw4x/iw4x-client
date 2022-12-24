#include <STDInclude.hpp>

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

	void Entities::deleteTriggers()
	{
		for (auto i = this->entities.begin(); i != this->entities.end();)
		{
			if (i->find("classname") != i->end())
			{
				std::string classname = (*i)["classname"];
				if (String::StartsWith(classname, "trigger_"))
				{
					i = this->entities.erase(i);
					continue;
				}
			}

			++i;
		}
	}

	void Entities::convertTurrets()
	{
		for (auto& entity : this->entities)
		{
			if (entity.contains("classname"))
			{
				if (entity["classname"] == "misc_turret"s)
				{
					entity["weaponinfo"] = "turret_minigun_mp";
					entity["model"] = "weapon_minigun";
				}
			}
		}
	}

	void Entities::deleteWeapons(bool keepTurrets)
	{
		for (auto i = this->entities.begin(); i != this->entities.end();)
		{
			if (i->find("weaponinfo") != i->end() || (i->find("targetname") != i->end() && (*i)["targetname"] == "oldschool_pickup"s))
			{
				if (!keepTurrets || i->find("classname") == i->end() || (*i)["classname"] != "misc_turret"s)
				{
					i = this->entities.erase(i);
					continue;
				}
			}

			++i;
		}
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
