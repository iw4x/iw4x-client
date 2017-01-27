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
			Channel(std::string _name, int queueSize = 100, int bufferSize = 20);
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

			void sendMessage(std::string data);

			std::unique_ptr<boost::interprocess::message_queue> queue;
			std::string packet;
			std::string name;
		};
	}
}
