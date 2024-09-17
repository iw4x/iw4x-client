#include <STDInclude.hpp>

#include "Field.hpp"
#include "Int64.hpp"
#include "IO.hpp"
#include "Script.hpp"
#include "ScriptError.hpp"
#include "ScriptExtension.hpp"
#include "ScriptPatches.hpp"
#include "ScriptStorage.hpp"
#include "String.hpp"
#include "UserInfo.hpp"

namespace Components::GSC
{
	GSC::GSC()
	{
		Loader::Register(new Field());
		Loader::Register(new Int64());
		Loader::Register(new IO());
		Loader::Register(new Script());
		Loader::Register(new ScriptError());
		Loader::Register(new ScriptExtension());
		Loader::Register(new ScriptPatches());
		Loader::Register(new ScriptStorage());
		Loader::Register(new String());
		Loader::Register(new UserInfo());
	}
}
