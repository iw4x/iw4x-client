#include <STDInclude.hpp>

#include "Int64.hpp"
#include "IO.hpp"
#include "Script.hpp"
#include "ScriptExtension.hpp"
#include "ScriptStorage.hpp"

namespace Components
{
	GSC::GSC()
	{
		Loader::Register(new Int64());
		Loader::Register(new IO());
		Loader::Register(new Script());
		Loader::Register(new ScriptExtension());
		Loader::Register(new ScriptStorage());
	}
}
