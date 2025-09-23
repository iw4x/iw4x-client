#include "GSC.hpp"
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
		static Field Field_{};
		static Int64 Int64_{};
		static IO IO_{};
		static Script Script_{};
		static ScriptError ScriptError_{};
		static ScriptExtension ScriptExtension_{};
		static ScriptPatches ScriptPatches_{};
		static ScriptStorage ScriptStorage_{};
		static String String_{};
		static UserInfo UserInfo_{};
	}
}
