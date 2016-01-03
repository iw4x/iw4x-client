#include "..\..\STDInclude.hpp"
#include <future>

namespace Components
{
	Discovery::Discovery()
	{
		Command::Add("bcast", [] (Command::Params params)
		{
			std::async([]()
			{			
				int start = Game::Com_Milliseconds();
				OutputDebugStringA("Start!");
				Network::BroadcastAll("getinfo xxx\n");
				OutputDebugStringA(Utils::VA("End: %d", Game::Com_Milliseconds() - start));
			});
		});
	}

	Discovery::~Discovery()
	{
		
	}
}
