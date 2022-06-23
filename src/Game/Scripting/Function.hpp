#pragma once

namespace Scripting
{
	class Function
	{
	public:
		Function(const char* pos);

		[[nodiscard]] const char* getPos() const;

	private:
		const char* pos_;
	};
}
