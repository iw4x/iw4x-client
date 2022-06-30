#pragma once

namespace Scripting
{
	class StackIsolation final
	{
	public:
		StackIsolation();
		~StackIsolation();

		StackIsolation(StackIsolation&&) = delete;
		StackIsolation(const StackIsolation&) = delete;
		StackIsolation& operator=(StackIsolation&&) = delete;
		StackIsolation& operator=(const StackIsolation&) = delete;

	private:
		Game::VariableValue stack_[512]{};

		Game::VariableValue* maxStack_;
		Game::VariableValue* top_;
		unsigned int inParamCount_;
		unsigned int outParamCount_;
	};
}
