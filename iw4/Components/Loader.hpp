namespace Components
{
	class Component
	{
	public:
		Component() {};
		virtual ~Component() {};
		virtual const char* GetName() { return "Unknown"; };
	};

	class Loader
	{
	public:
		static void Initialize();
		static void Uninitialize();
		static void Register(Component* component);

	private:
		static std::vector<Component*> Components;
	};
}

#include "Dvar.hpp"
#include "Colors.hpp"
#include "Window.hpp"
#include "Command.hpp"
#include "Console.hpp"
#include "RawFiles.hpp"
#include "QuickPatch.hpp"
