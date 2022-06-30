#include <STDInclude.hpp>

namespace Scripting
{
	StackIsolation::StackIsolation()
	{
		this->inParamCount_ = Game::scrVmPub->inparamcount;
		this->outParamCount_ = Game::scrVmPub->outparamcount;
		this->top_ = Game::scrVmPub->top;
		this->maxStack_ = Game::scrVmPub->maxStack;

		Game::scrVmPub->top = this->stack_;
		Game::scrVmPub->maxStack = &this->stack_[ARRAYSIZE(this->stack_) - 1];
		Game::scrVmPub->inparamcount = 0;
		Game::scrVmPub->outparamcount = 0;
	}

	StackIsolation::~StackIsolation()
	{
		Game::Scr_ClearOutParams();
		Game::scrVmPub->inparamcount = this->inParamCount_;
		Game::scrVmPub->outparamcount = this->outParamCount_;
		Game::scrVmPub->top = this->top_;
		Game::scrVmPub->maxStack = this->maxStack_;
	}
}
