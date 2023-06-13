#pragma once

#include <vector>
#include <thread>
#include <string>
#include <unordered_map>
#include <unordered_set>

#ifndef ServerVersion
#define ServerVersion "0.1"
#endif

namespace ContinuousDelivery {

	struct ProductInstalation {
		std::string InstalationFolder;
		int BuildNumber;
		std::string ProductRing; // E.g. Nightly, Alpha, Beta, Release
		std::string Family; // E.g. Standard Edition, Premium Edition
		std::string Platform; // Windows, Linux or MacOs
		std::string Architecture;// E.g. x64, x84 or ARM
		std::string ProductName;
	};

	class DeliveryServer {

		struct InstallationDetails {
			std::string InstalationDirectory;
			std::string FileManifest;
		};

		using BuildNumberMap = std::unordered_map<int, InstallationDetails>; // BuildNumber -> InstalationFolder
		using ArchitectureMap = std::unordered_map<std::string, BuildNumberMap>; // Achitecture -> BuildNumber
		using PlatformMap = std::unordered_map<std::string, ArchitectureMap>; // Platform -> Architecture
		using ProductRingMap = std::unordered_map<std::string, PlatformMap>; // ProductRing -> Platform
		using ProductFamilyMap = std::unordered_map<std::string, ProductRingMap>; // Product Family -> Product Ring
		using ProductNameMap = std::unordered_map<std::string, ProductFamilyMap>; // ProductName -> ProductFamily

		using ProductInstalationsMap = ProductNameMap;

		public:
			DeliveryServer() = delete;
			DeliveryServer(std::string hostname, int port);

			void StartServer();
			void Stop();

			void Update();

			void AddProductInstalation(ProductInstalation instalation);
			void SaveConfigFile();
			void LoadConfigFile();

			bool IsRunning();
			int Port();
			std::string HostName();
	private:
		void RegisterRoutes();

		std::string ProductInstallations();

		char lastChar = ' ';

		int _port;
		std::string _hostName;

		ProductInstalationsMap _productMap;
		std::vector<ProductInstalation> _instalations;

		bool _isStarted;
		std::jthread _serverThread;
	};

}
