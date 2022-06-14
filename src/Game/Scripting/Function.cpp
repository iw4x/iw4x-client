#include <STDInclude.hpp>

namespace Scripting
{
	Function::Function(const char* pos)
		: pos_(pos)
	{
	}

	const char* Function::getPos() const
	{
		return this->pos_;
	}
}
