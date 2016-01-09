#define FRAME_PACKET_LIMIT 20
#define DOWNLOAD_TIMEOUT 2500
#define PACKET_SIZE 1000

namespace Components
{
	class Download : public Component
	{
	public:
		struct Container
		{
			struct DownloadCL
			{
				int id;
				int startTime;
				int lastPing;
				bool acknowledged;
				int maxParts;
				Network::Address target;
				std::map<int, std::string> parts;
				std::function<void(int)> failureCallback;
				std::function<void(int, std::string)> successCallback;
			};

			struct DownloadSV
			{
				int id;
				int startTime;
				int lastPing;
				bool acknowledged;
				std::vector<int> sentParts;
				int maxParts;
				Network::Address target;
				std::string challenge;
				std::string buffer;
			};

			struct Packet
			{
				int id;
				int partId;
				int length;
				unsigned int hash;
			};

			struct AckRequest
			{
				int id;
				int maxPackets;
				int length;
			};

			std::vector<DownloadCL> ClientDownloads;
			std::vector<DownloadSV> ServerDownloads;
		};

		Download();
		~Download();
		const char* GetName() { return "Download"; };

		static int Get(Network::Address target, std::string file, std::function<void(int, std::string)> successCallback, std::function<void(int)> failureCallback);

		static Container::DownloadCL* FindClientDownload(int id);
		static Container::DownloadSV* FindServerDownload(int id);

	private:
		static void Frame();
		static Container DataContainer;

		// Client handlers
		static void AckRequest(Network::Address target, std::string data);
		static void PacketResponse(Network::Address target, std::string data);

		// Server handlers
		static void AckResponse(Network::Address target, std::string data);
		static void MissingRequest(Network::Address target, std::string data);
		static void DownloadRequest(Network::Address target, std::string data);

		// Helper functions
		static void RemoveClientDownload(int id);
		static void RemoveServerDownload(int id);

		static bool HasSentPacket(Container::DownloadSV* download, int packet);
		static bool HasReceivedPacket(Container::DownloadCL* download, int packet);
		static bool HasReceivedAllPackets(Container::DownloadCL* download);
		static std::string AssembleBuffer(Container::DownloadCL* download);
		static void RequestMissingPackets(Container::DownloadCL* download, std::vector<int> packets);

		static void MarkPacketAsDirty(Container::DownloadSV* download, int packet);
		static void SendPacket(Container::DownloadSV* download, int packet);
		static int ReadPacketId(std::string &data);
	};
}
