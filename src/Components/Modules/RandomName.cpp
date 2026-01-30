#include "RandomName.hpp"
#include "Dvar.hpp"
#include "Events.hpp"
#include "Dedicated.hpp"
#include "ZoneBuilder.hpp"

#include <Utils/WebIO.hpp>

#include "rapidjson/document.h"

#include <random>

namespace Components
{
	void RandomName::FetchAndSetRandomName()
	{
		std::thread([]
		{
			try
			{
				bool success = false;
				const auto response = Utils::WebIO("IW4x", "https://randomuser.me/api/").setTimeout(5000)->get(&success);

				if (!success || response.empty())
				{
					Logger::Print("RandomName: Failed to fetch random name from API\n");
					return;
				}

				rapidjson::Document doc;
				const rapidjson::ParseResult parseResult = doc.Parse(response);

				if (!parseResult || !doc.IsObject())
				{
					Logger::Print("RandomName: Failed to parse API response\n");
					return;
				}

				if (!doc.HasMember("results") || !doc["results"].IsArray() || doc["results"].Empty())
				{
					Logger::Print("RandomName: Invalid API response structure\n");
					return;
				}

				const auto& result = doc["results"][0];

				std::string firstName, lastName, password, username;

				if (result.HasMember("name") && result["name"].IsObject())
				{
					const auto& name = result["name"];
					if (name.HasMember("first") && name["first"].IsString())
						firstName = name["first"].GetString();
					if (name.HasMember("last") && name["last"].IsString())
						lastName = name["last"].GetString();
				}

				if (result.HasMember("login") && result["login"].IsObject())
				{
					const auto& login = result["login"];
					if (login.HasMember("password") && login["password"].IsString())
						password = login["password"].GetString();
					if (login.HasMember("username") && login["username"].IsString())
						username = login["username"].GetString();
				}

				// Generate random number 1-100
				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_int_distribution<> dis(1, 100);
				const int roll = dis(gen);

				std::string selectedName;

				// 10% first name, 10% last name, 30% password, 50% username
				if (roll <= 10 && !firstName.empty())
				{
					selectedName = firstName;
				}
				else if (roll <= 20 && !lastName.empty())
				{
					selectedName = lastName;
				}
				else if (roll <= 50 && !password.empty())
				{
					selectedName = password;
				}
				else if (!username.empty())
				{
					selectedName = username;
				}
				else
				{
					// Fallback chain
					if (!username.empty()) selectedName = username;
					else if (!password.empty()) selectedName = password;
					else if (!firstName.empty()) selectedName = firstName;
					else if (!lastName.empty()) selectedName = lastName;
				}

				if (selectedName.empty())
				{
					Logger::Print("RandomName: No valid name found in API response\n");
					return;
				}

				// Schedule setting the name on the main thread
				Scheduler::Once([selectedName]
				{
					Dvar::Name.set(selectedName);
					Logger::Print("RandomName: Set name to '{}'\n", selectedName);
				}, Scheduler::Pipeline::CLIENT);
			}
			catch (const std::exception& e)
			{
				Logger::Print("RandomName: Exception occurred: {}\n", e.what());
			}
		}).detach();
	}

	RandomName::RandomName()
	{
		if (ZoneBuilder::IsEnabled() || Dedicated::IsEnabled())
		{
			return;
		}

		// Set random name on client initialization
		Events::OnClientInit([]
		{
			FetchAndSetRandomName();
		});

		// Set random name when disconnecting from a server
		Events::OnCLDisconnected([](bool wasConnected)
		{
			if (wasConnected)
			{
				FetchAndSetRandomName();
			}
		});
	}
}
