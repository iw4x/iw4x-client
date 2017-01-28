#pragma once

namespace boost
{
	typedef unsigned long long ulong_long_type;
}

#pragma warning(push)
#pragma warning(disable: 4091)
#pragma warning(disable: 4996)
#define _SCL_SECURE_NO_WARNINGS

#ifdef sleep
#undef sleep
#endif

//#define BOOST_DISABLE_WIN32
#define BOOST_USE_WINDOWS_H
#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/ipc/message_queue.hpp>

#undef _SCL_SECURE_NO_WARNINGS
#pragma warning(pop)

namespace Utils
{
	namespace IPC
	{
		class Channel
		{
		public:
			Channel(std::string _name, int _queueSize = 100, int _bufferSize = 1024, bool _remove = false);
			~Channel();

			bool receive(std::string* data);
			void send(std::string data);

		private:
			struct Header
			{
				bool fragmented;
				unsigned short packetId;
				size_t fragmentSize;
				size_t totalSize;
				unsigned int fragmentPart;
			};

			void enqueueMessage(std::string data);
			void queueWorker();

			bool terminateQueue;
			std::condition_variable queueEvent;
			std::mutex queueMutex;
			std::thread queueThread;
			std::queue<std::string> internalQueue;

			bool remove;
			std::unique_ptr<boost::interprocess::message_queue> queue;
			std::string packet;
			std::string name;
		};

		class BidirectionalChannel
		{
		public:
			BidirectionalChannel(std::string name, bool server, int queueSize = 100, int bufferSize = 1024) : isServer(server),
			channel1(name, queueSize, bufferSize, server),
			channel2(name + "2", queueSize, bufferSize, server)
			{}

			bool receive(std::string* data)
			{
				if(this->isServer)
				{
					return channel1.receive(data);
				}
				else
				{
					return channel2.receive(data);
				}
			}

			void send(std::string data)
			{
				if (this->isServer)
				{
					return channel2.send(data);
				}
				else
				{
					return channel1.send(data);
				}
			}

		private:
			const bool isServer;
			Channel channel1;
			Channel channel2;
		};
	}
}
