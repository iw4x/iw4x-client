#pragma once

namespace Components
{
	class ConnectProtocol
	{
	public:
		ConnectProtocol();

		static bool IsEvaluated();
		static bool Used();

	private:
		static bool Evaluated;
		static std::string ConnectString;

		static void EvaluateProtocol();
		static bool InstallProtocol();

		static void Invocation();
	};
}
