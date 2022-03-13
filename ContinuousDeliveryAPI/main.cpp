#include "DeliveryServer.h"
#include "PackageCollector.h"

#include "httplib.h" // https://github.com/yhirose/cpp-httplib

#include <filesystem>

ContinuousDelivery::DeliveryServer server("0.0.0.0", 8080);

int main() {

	std::cout << "Starting!\n";
	//server.AddProductInstalation({
	//	.InstalationFolder = std::filesystem::current_path().string() + "\\Test",
	//	.BuildNumber = 1,
	//	.ProductRing = "Test",
	//	.Family = "TestFamily",
	//	.ProductName = "TestProduct"
	//	}
	//);

	//server.AddProductInstalation({
	//	.InstalationFolder = std::filesystem::current_path().string() + "\\Test2",
	//	.BuildNumber = 2,
	//	.ProductRing = "Test",
	//	.Family = "TestFamily",
	//	.ProductName = "TestProduct"
	//	}
	//);

	//server.AddProductInstalation({
	//	.InstalationFolder = std::filesystem::current_path().string() + "\\Test3",
	//	.BuildNumber = 1,
	//	.ProductRing = "Dev",
	//	.Family = "TestFamily",
	//	.ProductName = "TestProduct"
	//	}
	//);

	//server.AddProductInstalation({
	//	.InstalationFolder = std::filesystem::current_path().string() + "\\Test4",
	//	.BuildNumber = 1,
	//	.ProductRing = "Dev",
	//	.Family = "TestFamily",
	//	.ProductName = "TestProduct2"
	//	}
	//);

	server.StartServer();

	while (server.IsRunning()) {
		server.Update();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	return 0;
}