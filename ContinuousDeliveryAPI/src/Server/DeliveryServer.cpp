#include "DeliveryServer.h"
#include "PackageCollector.h"

#include "httplib.h"  // https://github.com/yhirose/cpp-httplib

#include <yaml-cpp/yaml.h>

#include <fstream>
#include <filesystem>

httplib::Server server;
std::jthread inputThread;

#define DeliveryServerConfigFile "DeliveryServer.config"
#define DeliveryServerAdminToken "84UUhGwXfmdWXECEwRkuG4Kf6rmqFT"

namespace ContinuousDelivery {

	DeliveryServer::DeliveryServer(std::string hostname, int port)
		: _isStarted(false),
		_port(port),
		_hostName(hostname),
		_instalations()
	{
		server.set_keep_alive_max_count(500);
		server.set_logger([](const httplib::Request& req, const httplib::Response& res) {
			std::stringstream ss;
			ss << "\r" << req.path << ": " << res.status << "\n>>> ";
			std::cout << ss.str();
			});
	}

	void DeliveryServer::StartServer()
	{
		if (!_isStarted) {

			_isStarted = true;
			
			_serverThread = std::jthread([_hostName = HostName(), _port = Port()]()
			{
				server.listen(_hostName.c_str(), _port);
			});

			LoadConfigFile();
			RegisterRoutes();

			while (!server.is_running()) {
				std::cout << "\rServer Starting";
			}


			inputThread = std::jthread([this] {
				while (true) {
					std::cout << "\r>>> ";
					std::cin >> this->lastChar;
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				});

			std::cout << "\rOK!, server is running!\n";
			std::cout << "Press q to quit\n";
		}
	}

	bool DeliveryServer::IsRunning()
	{
		return _isStarted;
	}

	int DeliveryServer::Port()
	{
		return _port;
	}

	std::string DeliveryServer::HostName()
	{
		return _hostName;
	}

	void DeliveryServer::Stop()
	{
		if (_isStarted)
			server.stop();
	}

	void DeliveryServer::Update()
	{
		if (lastChar == 'q') {
			server.stop();
			inputThread.detach();
			_isStarted = false;
		}
	}

	void DeliveryServer::AddProductInstalation(ProductInstalation instalation)
	{
		if (!_productMap.contains(instalation.ProductName))
			_productMap.emplace(instalation.ProductName, ProductFamilyMap());

		ProductFamilyMap& families = _productMap.at(instalation.ProductName);
		if (!families.contains(instalation.Family))
			families.emplace(instalation.Family, ProductRingMap());

		ProductRingMap& productRings = families.at(instalation.Family);
		if (!productRings.contains(instalation.ProductRing))
			productRings.emplace(instalation.ProductRing, PlatformMap());

		PlatformMap& platforms = productRings.at(instalation.ProductRing);
		if (!platforms.contains(instalation.Platform))
			platforms.emplace(instalation.Platform, ArchitectureMap());

		ArchitectureMap& achitectures = platforms.at(instalation.Platform);
		if (!achitectures.contains(instalation.Architecture))
			achitectures.emplace(instalation.Architecture, BuildNumberMap());

		BuildNumberMap& buildNumbers = achitectures.at(instalation.Architecture);
		if (!buildNumbers.contains(instalation.BuildNumber)) {

			ContinuousDelivery::PackageCollector collector(instalation.InstalationFolder);
			InstallationDetails details{
				.InstalationDirectory = instalation.InstalationFolder,
				.FileManifest = collector.Assemble()
			};

			buildNumbers.emplace(instalation.BuildNumber, details);
			_instalations.emplace_back(instalation);

			std::stringstream route;
			route << instalation.ProductName;
			route << "/";
			route << instalation.Family;
			route << "/";
			route << instalation.ProductRing;
			route << "/";
			route << instalation.Platform;
			route << "/";
			route << instalation.Architecture;
			route << "/";
			route << instalation.BuildNumber;

			std::cout << "Registered Manifest route: /Manifest/" + route.str() + "\n";
			server.Get("/Manifest/" + route.str(), [&details = buildNumbers[instalation.BuildNumber]](const httplib::Request&, httplib::Response& res) {
				res.set_content(details.FileManifest, "text/plain");
			});
			std::cout << "Registered Files route: /Files/" + route.str() + "\n";
			server.set_mount_point("/Files/" + route.str(), details.InstalationDirectory.c_str());
		}


	}

	void DeliveryServer::SaveConfigFile()
	{
		YAML::Emitter emitter;
		emitter << YAML::BeginMap << YAML::Key << "DeliveryServerConfig";
		emitter << YAML::Value << YAML::BeginSeq;
		for (const auto& productInstall : _instalations) {
			emitter << YAML::BeginMap;
			emitter << YAML::Key << "ProductName" << YAML::Value << productInstall.ProductName;
			emitter << YAML::Key << "InstallDir" << YAML::Value << productInstall.InstalationFolder;
			emitter << YAML::Key << "Family" << YAML::Value << productInstall.Family;
			emitter << YAML::Key << "Ring" << YAML::Value << productInstall.ProductRing;
			emitter << YAML::Key << "Platform" << YAML::Value << productInstall.Platform;
			emitter << YAML::Key << "Architecture" << YAML::Value << productInstall.Architecture;
			emitter << YAML::Key << "BuildNumber" << YAML::Value << productInstall.BuildNumber;
			emitter << YAML::EndMap;
		}
		emitter << YAML::EndSeq;
		emitter << YAML::EndMap;

		std::string outputFileContents(emitter.c_str());
		std::fstream configFile(DeliveryServerConfigFile, std::ios::out | std::ios::trunc);
		configFile.write(outputFileContents.c_str(), outputFileContents.size());
		configFile.close();
	}

	void DeliveryServer::LoadConfigFile()
	{
		if(!std::filesystem::exists(DeliveryServerConfigFile))
			return;

		std::ifstream configFile(DeliveryServerConfigFile, std::ios::in);
		std::stringstream buffer;
		buffer << configFile.rdbuf();
		configFile.close();

		YAML::Node configYAML = YAML::Load(buffer.str())["DeliveryServerConfig"];
		for (YAML::const_iterator it = configYAML.begin(); it != configYAML.end(); ++it) {
			const YAML::Node& installSequenceValue = *it;

			ProductInstalation install;
			install.InstalationFolder = installSequenceValue["InstallDir"].as<std::string>();
			install.ProductName = installSequenceValue["ProductName"].as<std::string>();
			install.Family = installSequenceValue["Family"].as<std::string>();
			install.ProductRing = installSequenceValue["Ring"].as<std::string>();
			install.Platform = installSequenceValue["Platform"].as<std::string>();
			install.Architecture = installSequenceValue["Architecture"].as<std::string>();
			install.BuildNumber = installSequenceValue["BuildNumber"].as<int>();

			AddProductInstalation(install);
		}
	}

	void DeliveryServer::RegisterRoutes()
	{
		server.Get("/Version", [](const httplib::Request&, httplib::Response& res) {
			res.set_content(ServerVersion, "text/plain");
			}
		);

		server.Get("/Installations", [this](const httplib::Request&, httplib::Response& res) {
			res.set_content(ProductInstallations(), "text/plain");
			}
		);

		server.Post("/AddInstalation", [this](const httplib::Request& req, httplib::Response& res) {
			if (!req.has_header("AdminToken")) {

				res.status = 400;
				return;
			}

			if (req.get_header_value("AdminToken") != DeliveryServerAdminToken) {

				res.status = 400;
				return;
			}
			
			if (!req.has_param("InstallDir")
				|| !req.has_param("ProductName")
				|| !req.has_param("Ring")
				|| !req.has_param("Family")
				|| !req.has_param("Platform")
				|| !req.has_param("Architecture")
				|| !req.has_param("BuildNumber")) {
				res.status = 400;
				return;
			}

			ProductInstalation install{
				.InstalationFolder = req.get_param_value("InstallDir"),
				.BuildNumber = std::stoi(req.get_param_value("BuildNumber")),
				.ProductRing = req.get_param_value("Ring"),
				.Family = req.get_param_value("Family"),
				.Platform = req.get_param_value("Platform"),
				.Architecture = req.get_param_value("Architecture"),
				.ProductName = req.get_param_value("InstallDir"),
			};

			if (!std::filesystem::exists(install.InstalationFolder))
			{
				res.status = 400;
				return;
			}

			AddProductInstalation(install);
			SaveConfigFile();

			res.set_content("Ok!", "text/plain");
			}
		);
	}

	std::string DeliveryServer::ProductInstallations()
	{
		YAML::Emitter emitter;
		emitter << YAML::BeginSeq;
		for (const auto& productIterator : _productMap) {
			const std::string& productName = productIterator.first;

			emitter << YAML::BeginMap;
			emitter << YAML::Key << "Product" << YAML::Value << productName;
			emitter << YAML::Key << "Families" << YAML::Value << YAML::BeginSeq;
			for (const auto& familiesIterator : productIterator.second) {
				const std::string& family = familiesIterator.first;
				emitter << YAML::BeginMap;
				emitter << YAML::Key << "Family" << YAML::Value << family;
				emitter << YAML::Key << "Rings" << YAML::Value << YAML::BeginSeq;
				for (const auto& ringIterator : familiesIterator.second) {
					const std::string& ring = ringIterator.first;
					emitter << YAML::BeginMap;
					emitter << YAML::Key << "Ring" << YAML::Value << ring;
					emitter << YAML::Key << "Platforms" << YAML::Value << YAML::BeginSeq;
					for (const auto& platformIterator : ringIterator.second) {
						const std::string& platform = platformIterator.first;
						emitter << YAML::BeginMap;
						emitter << YAML::Key << "Platform" << YAML::Value << platform;
						emitter << YAML::Key << "Architectures" << YAML::Value << YAML::BeginSeq;
						for (const auto& achitectureIterator : platformIterator.second) {
							const std::string& architecture = achitectureIterator.first;
							emitter << YAML::BeginMap;
							emitter << YAML::Key << "Architecture" << YAML::Value << architecture;
							emitter << YAML::Key << "BuildNumbers" << YAML::Value << YAML::BeginSeq;
							for (const auto& buildIterator : achitectureIterator.second) {
								const int& buildNumber = buildIterator.first;
								emitter << buildNumber;
							}
							emitter << YAML::EndSeq;
							emitter << YAML::EndMap;
						}
						emitter << YAML::EndSeq;
						emitter << YAML::EndMap;
					}
					emitter << YAML::EndSeq;
					emitter << YAML::EndMap;
				}
				emitter << YAML::EndSeq;
				emitter << YAML::EndMap;
			}
			emitter << YAML::EndSeq;
			emitter << YAML::EndMap;
		}
		emitter << YAML::EndSeq;

		return std::string(emitter.c_str());
	}
}
