#include "STDInclude.hpp"

namespace Utils
{
	namespace IPC
	{
		Channel::Channel(std::string _name, int _queueSize, int _bufferSize, bool _remove) : name(_name), remove(_remove)
		{
			if(this->remove) boost::interprocess::message_queue::remove(this->name.data());
			queue.reset(new boost::interprocess::message_queue(boost::interprocess::open_or_create, this->name.data(), _queueSize, _bufferSize + sizeof(Channel::Header)));
		}

		Channel::~Channel()
		{
			if (this->remove) boost::interprocess::message_queue::remove(this->name.data());
		}

		bool Channel::receive(std::string* data)
		{
			if (!data) return false;
			data->clear();

			if(this->packet.size() < this->queue->get_max_msg_size())
			{
				packet.resize(this->queue->get_max_msg_size());
			}
			
			const Channel::Header* header = reinterpret_cast<const Channel::Header*>(packet.data());
			const char* buffer = reinterpret_cast<const char*>(header + 1);

			unsigned int priority;
			boost::interprocess::message_queue::size_type recvd_size;

			if (this->queue->try_receive(const_cast<char*>(packet.data()), packet.size(), recvd_size, priority))
			{
				if ((recvd_size - sizeof(Channel::Header)) != header->fragmentSize || header->fragmentPart) return false;
				data->append(buffer, header->fragmentSize);

				if(header->fragmented)
				{
					Channel::Header mainHeader = *header;
					unsigned int part = mainHeader.fragmentPart;

					while (true)
					{
						if (!this->queue->try_receive(const_cast<char*>(packet.data()), packet.size(), recvd_size, priority)) return false;

						if (header->packetId != mainHeader.packetId || header->totalSize != mainHeader.totalSize || header->fragmentPart != (++part))
						{
							data->clear();
							return false;
						}

						data->append(buffer, header->fragmentSize);

						if(header->totalSize <= data->size())
						{
							break;
						}
					}
				}

				return true;
			}

			return false;
		}

		void Channel::send(std::string data)
		{
			const size_t fragmentSize = (this->queue->get_max_msg_size() - sizeof(Channel::Header)) - 1;

			Channel::Header header;
			header.fragmented = (data.size() + sizeof(Channel::Header)) > this->queue->get_max_msg_size();
			header.packetId = static_cast<short>(timeGetTime() + rand());;
			header.totalSize = data.size();

			size_t sentSize = 0;
			for (unsigned short i = 0; sentSize < data.size(); ++i)
			{
				header.fragmentPart = i;
				header.fragmentSize = std::min(fragmentSize, data.size() - (fragmentSize * i));

				std::string buffer;
				buffer.append(reinterpret_cast<char*>(&header), sizeof(Channel::Header));
				buffer.append(data.data() + sentSize, header.fragmentSize);
				Channel::sendMessage(buffer);

				sentSize += header.fragmentSize;
			}
		}

		void Channel::sendMessage(std::string data)
		{
			if (data.size() <= this->queue->get_max_msg_size())
			{
				while (!this->queue->try_send(data.data(), data.size(), 0))
				{
					std::this_thread::sleep_for(100us);
				}
			}
		}
	}
}
