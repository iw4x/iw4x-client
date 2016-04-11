#include "STDInclude.hpp"

namespace Components
{
	Download::Container Download::DataContainer;

	Download::Container::DownloadCL* Download::FindClientDownload(int id)
	{
		for (auto &download : Download::DataContainer.ClientDownloads)
		{
			if (download.id == id)
			{
				return &download;
			}
		}
		
		return nullptr;
	}

	Download::Container::DownloadSV* Download::FindServerDownload(int id)
	{
		for (auto &download : Download::DataContainer.ServerDownloads)
		{
			if (download.id == id)
			{
				return &download;
			}
		}

		return nullptr;
	}

	void Download::RemoveClientDownload(int id)
	{
		for (auto i = Download::DataContainer.ClientDownloads.begin(); i != Download::DataContainer.ClientDownloads.end(); ++i)
		{
			if (i->id == id)
			{
				Download::DataContainer.ClientDownloads.erase(i);
				return;
			}
		}
	}

	void Download::RemoveServerDownload(int id)
	{
		for (auto i = Download::DataContainer.ServerDownloads.begin(); i != Download::DataContainer.ServerDownloads.end(); ++i)
		{
			if (i->id == id)
			{
				Download::DataContainer.ServerDownloads.erase(i);
				return;
			}
		}
	}

	bool Download::HasSentPacket(Download::Container::DownloadSV* download, int packet)
	{
		for (auto sentPacket : download->sentParts)
		{
			if (packet == sentPacket)
			{
				return true;
			}
		}

		return false;
	}

	bool Download::HasReceivedPacket(Download::Container::DownloadCL* download, int packet)
	{
		if (!download->parts.empty())
		{
			for (auto i = download->parts.begin(); i != download->parts.end(); ++i)
			{
				if (i->first == packet)
				{
					return true;
				}
			}
		}

		return false;
	}

	bool Download::HasReceivedAllPackets(Download::Container::DownloadCL* download)
	{
		for (int i = 0; i < download->maxParts; ++i)
		{
			if (!Download::HasReceivedPacket(download, i))
			{
				return false;
			}
		}

		return true;
	}

	int Download::ReadPacketId(std::string &data)
	{
		int id = *(int*)data.data();
		data = std::string(data.data() + sizeof(int), data.size() - sizeof(int));
		return id;
	}

	// Client handlers
	void Download::AckRequest(Network::Address target, std::string data)
	{
		if (data.size() < sizeof(Download::Container::AckRequest)) return; // Drop invalid packets, if they were important, we'll re-request them later
		Download::Container::AckRequest* request = (Download::Container::AckRequest*)data.data();

		if (data.size() < (sizeof(Download::Container::AckRequest) + request->length)) return; // Again, drop invalid packets

		auto download = Download::FindClientDownload(request->id);

		if (download && download->target == target && !download->acknowledged)
		{
			std::string challenge(data.data() + sizeof(Download::Container::AckRequest), request->length);

			download->acknowledged = true;
			download->lastPing = Game::Com_Milliseconds();
			download->maxParts = request->maxPackets;

			std::string packet;
			packet.append(reinterpret_cast<char*>(&download->id), sizeof(int));
			packet.append(challenge);

			Network::SendCommand(target, "dlAckResponse", packet);
		}
	}

	void Download::PacketResponse(Network::Address target, std::string data)
	{
		//Logger::Print("Packet incoming!\n");
		if (data.size() < sizeof(Download::Container::Packet)) return; // Drop invalid packets, if they were important, we'll re-request them later
		Download::Container::Packet* packet = (Download::Container::Packet*)data.data();
		//Logger::Print("Reading data!\n");
		if (data.size() < (sizeof(Download::Container::Packet) + packet->length)) return; // Again, drop invalid packets
		//Logger::Print("Finding corresponding download!\n");
		auto download = Download::FindClientDownload(packet->id);

		if (download && download->target == target)
		{
			//Logger::Print("Parsing packet!\n");
			download->lastPing = Game::Com_Milliseconds();
			std::string packetData(data.data() + sizeof(Download::Container::Packet), packet->length);

			if (packet->hash == Utils::Cryptography::JenkinsOneAtATime::Compute(packetData.data(), packetData.size()))
			{
				//Logger::Print("Packet added!\n");
				download->parts[packet->partId] = packetData;

				if (Download::HasReceivedAllPackets(download))
				{
					download->successCallback(download->id, Download::AssembleBuffer(download));
					Download::RemoveClientDownload(download->id);
				}
			}
			else
			{
				Logger::Print("Hash invalid!\n");
			}
		}
	}


	// Server handlers
	void Download::AckResponse(Network::Address target, std::string data)
	{
		int id = Download::ReadPacketId(data);
		std::string challenge = Utils::ParseChallenge(data); // TODO: Maybe optimize this to ensure length matches

		auto download = Download::FindServerDownload(id);

		if (download && download->target == target)
		{
			if (download->challenge != challenge)
			{
				Logger::Print("Invalid download challenge!\n");
				Download::RemoveServerDownload(id);
			}
			else
			{
				download->lastPing = Game::Com_Milliseconds();
				download->acknowledged = true;
				Logger::Print("Client acknowledged!\n");
			}
		}
	}

	void Download::MissingRequest(Network::Address target, std::string data)
	{
		int id = Download::ReadPacketId(data);

		auto download = Download::FindServerDownload(id);

		if (download && download->target == target)
		{
			while ((data.size() % 4) >= 4)
			{
				Download::MarkPacketAsDirty(download, *reinterpret_cast<int*>(const_cast<char*>(data.data())));
				data = data.substr(4);
			}
		}
	}

	void Download::DownloadRequest(Network::Address target, std::string data)
	{
		int id = Download::ReadPacketId(data);

		Download::Container::DownloadSV download;
		download.id = id;
		download.target = target;
		download.acknowledged = false;
		download.startTime = Game::Com_Milliseconds();
		download.lastPing = Game::Com_Milliseconds();
		download.maxParts = 0;

		for (int i = 0; i < 1000000; ++i)
		{
			download.buffer.append("1234567890");
		}

		download.maxParts = download.buffer.size() / PACKET_SIZE;
		if (download.buffer.size() % PACKET_SIZE) download.maxParts++;

		download.challenge = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());

		Download::Container::AckRequest request;
		request.id = id;
		request.maxPackets = download.maxParts;
		request.length = download.challenge.size();


		std::string packet;
		packet.append(reinterpret_cast<char*>(&request), sizeof(request));
		packet.append(download.challenge);

		Download::DataContainer.ServerDownloads.push_back(download);

		Network::SendCommand(target, "dlAckRequest", packet);
	}

	std::string Download::AssembleBuffer(Download::Container::DownloadCL* download)
	{
		std::string buffer;

		for (int i = 0; i < download->maxParts; ++i)
		{
			if (!Download::HasReceivedPacket(download, i)) return "";
			buffer.append(download->parts[i]);
		}

		return buffer;
	}

	void Download::RequestMissingPackets(Download::Container::DownloadCL* download, std::vector<int> packets)
	{
		if (!packets.empty())
		{
			download->lastPing = Game::Com_Milliseconds();

			std::string data;
			data.append(reinterpret_cast<char*>(&download->id), sizeof(int));

			for (auto &packet : packets)
			{
				data.append(reinterpret_cast<char*>(&packet), sizeof(int));
			}

			Network::SendCommand(download->target, "dlMissRequest", data);
		}
	}

	void Download::MarkPacketAsDirty(Download::Container::DownloadSV* download, int packet)
	{
		if (!download->sentParts.empty())
		{
			for (auto i = download->sentParts.begin(); i != download->sentParts.end(); ++i)
			{
				if (*i == packet)
				{
					download->sentParts.erase(i);
					i = download->sentParts.begin();
				}
			}
		}
	}

	void Download::SendPacket(Download::Container::DownloadSV* download, int packet)
	{
		if (!download || packet >= download->maxParts) return;
		download->lastPing = Game::Com_Milliseconds();
		download->sentParts.push_back(packet);

		Download::Container::Packet packetContainer;
		packetContainer.id = download->id;
		packetContainer.partId = packet;

		int size = ((packet + 1) == download->maxParts ? (download->buffer.size() % PACKET_SIZE) : PACKET_SIZE);
		size = (size == 0 ? PACKET_SIZE : size); // If remaining data equals packet PACKET_SIZE, size would be 0, so adjust it.
		std::string data(download->buffer.data() + (packet * PACKET_SIZE), size);

		packetContainer.length = data.size();
		packetContainer.hash = Utils::Cryptography::JenkinsOneAtATime::Compute(data.data(), data.size());

		std::string response = "dlPacketResponse\n";
		response.append(reinterpret_cast<char*>(&packetContainer), sizeof(packetContainer));
		response.append(data);

		Network::SendCommand(download->target, "dlPacketResponse", response);
	}

	void Download::Frame()
	{
		if (!Download::DataContainer.ClientDownloads.empty())
		{
			for (auto i = Download::DataContainer.ClientDownloads.begin(); i != Download::DataContainer.ClientDownloads.end(); ++i)
			{
				if ((Game::Com_Milliseconds() - i->lastPing) > (DOWNLOAD_TIMEOUT * 2))
				{
					i->failureCallback(i->id);
					Download::DataContainer.ClientDownloads.erase(i);
					return;
				}

				// Request missing parts
				if (i->acknowledged && (Game::Com_Milliseconds() - i->lastPing) > DOWNLOAD_TIMEOUT)
				{
					std::vector<int> missingPackets;
					for (int j = 0; j < i->maxParts; ++j)
					{
						if (!Download::HasReceivedPacket(&(*i), j))
						{
							missingPackets.push_back(j);
						}
					}

					Download::RequestMissingPackets(&(*i), missingPackets);
				}
			}
		}

		if (!Download::DataContainer.ServerDownloads.empty())
		{
			for (auto i = Download::DataContainer.ServerDownloads.begin(); i != Download::DataContainer.ServerDownloads.end(); ++i)
			{
				if ((Game::Com_Milliseconds() - i->lastPing) > (DOWNLOAD_TIMEOUT * 3))
				{
					Download::DataContainer.ServerDownloads.erase(i);
					return;
				}

				int packets = 0;
				for (int j = 0; j < i->maxParts && packets <= FRAME_PACKET_LIMIT && i->acknowledged; ++j)
				{
					if (!Download::HasSentPacket(&(*i), j))
					{
						//Logger::Print("Sending packet...\n");
						Download::SendPacket(&(*i), j);
						packets++;
					}
				}
			}
		}
	}

	int Download::Get(Network::Address target, std::string file, std::function<void(int, std::string)> successCallback, std::function<void(int)> failureCallback)
	{
		Download::Container::DownloadCL download;
		download.id = Game::Com_Milliseconds();
		download.target = target;
		download.acknowledged = false;
		download.startTime = Game::Com_Milliseconds();
		download.lastPing = Game::Com_Milliseconds();
		download.maxParts = 0;

		download.failureCallback = failureCallback;
		download.successCallback = successCallback;

		Download::DataContainer.ClientDownloads.push_back(download);

		std::string response = "dlRequest\n";
		response.append(reinterpret_cast<char*>(&download.id), sizeof(int));
		response.append(file);

		Network::SendCommand(target, "dlRequest", response);

		return download.id;
	}

	Download::Download()
	{
#ifdef ENABLE_EXPERIMENTAL_UDP_DOWNLOAD
		// Frame handlers
		QuickPatch::OnFrame(Download::Frame);

		// Register client handlers
		Network::Handle("dlAckRequest", Download::AckRequest);
		Network::Handle("dlPacketResponse", Download::PacketResponse);

		// Register server handlers
		Network::Handle("dlAckResponse", Download::AckResponse);
		Network::Handle("dlMissRequest", Download::MissingRequest);
		Network::Handle("dlAckResponse", Download::AckResponse);
		Network::Handle("dlRequest", Download::DownloadRequest);

		Command::Add("zob", [] (Command::Params params)
		{
			Logger::Print("Requesting!\n");
			Download::Get(Network::Address("192.168.0.23:28960"), "test", [] (int id, std::string data)
			{
				Logger::Print("Download succeeded %d!\n", Game::Com_Milliseconds() - (Download::FindClientDownload(id)->startTime));
			}, [] (int id)
			{
				Logger::Print("Download failed!\n");
			});
		});
#endif
	}

	Download::~Download()
	{
		Download::DataContainer.ServerDownloads.clear();
		Download::DataContainer.ClientDownloads.clear();
	}
}
