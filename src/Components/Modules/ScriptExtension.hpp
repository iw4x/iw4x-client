#pragma once

namespace Components
{
	class ScriptExtension : public Component
	{
	public:
		ScriptExtension();

	private:

		static void AddFunctions();
		static void AddMethods();
	};
}
