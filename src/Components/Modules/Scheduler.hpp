#pragma once

namespace Components
{
	class Scheduler : public Component
	{
	public:
		enum class Pipeline : int
		{
			ASYNC,
			RENDERER,
			SERVER,
			CLIENT,
			MAIN,
			QUIT,
			COUNT,
		};

		Scheduler();

		void preDestroy() override;

		static void Schedule(const std::function<bool()>& callback, Pipeline type,
			std::chrono::milliseconds delay = 0ms);
		static void Loop(const std::function<void()>& callback, Pipeline type,
			std::chrono::milliseconds delay = 0ms);
		static void Once(const std::function<void()>& callback, Pipeline type,
			std::chrono::milliseconds delay = 0ms);
		static void OnGameInitialized(const std::function<void()>& callback, Pipeline type,
			std::chrono::milliseconds delay = 0ms);
		static void OnGameShutdown(const std::function<void()>& callback);

	private:
		struct Task
		{
			std::function<bool()> handler{};
			std::chrono::milliseconds interval{};
			std::chrono::high_resolution_clock::time_point lastCall{};
		};

		using taskList = std::vector<Task>;

		class TaskPipeline
		{
		public:
			void add(Task&& task);
			void execute();

		private:
			Utils::Concurrency::Container<taskList> newCallbacks_;
			Utils::Concurrency::Container<taskList, std::recursive_mutex> callbacks_;

			void mergeCallbacks();
		};

		static volatile bool Kill;
		static std::thread Thread;
		static TaskPipeline Pipelines[];

		static void Execute(Pipeline type);

		static void ScrPlace_EndFrame_Hk();
		static void ServerFrame_Hk();
		static void ClientFrame_Hk(int localClientNum);
		static void MainFrame_Hk();
		static void SysSetBlockSystemHotkeys_Hk(int block);
	};
}
