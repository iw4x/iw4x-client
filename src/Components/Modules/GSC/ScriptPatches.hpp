#pragma once

namespace Components::GSC
{
	class ScriptPatches : public Component
	{
	public:
		ScriptPatches();

	private:
		static void Scr_TableLookupIStringByRow_Hk();
	};
}
