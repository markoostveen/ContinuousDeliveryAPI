
#include "httplib.h"

#include "FileHashes/FileHashes.h"

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

std::string InstalationDir = std::filesystem::current_path().string();

std::string GetRequest(std::string url) {
    try
    {
        httplib::Client client("http://localhost:8080");
        
        // send a get request
        const auto response = client.Get(url.c_str());
        return response->body;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Request failed, error: " << e.what() << '\n';
        return "";
    }
}

void RequestInstalation(std::string url,std::string localInstallPath) {
    httplib::Client client("http://localhost:8080");

    client.set_keep_alive(true);

    std::cout << "Checking Version\n";
    std::cout << client.Get("/Version")->body << "\n";

    std::cout << "Getting available products\n";
    std::cout << client.Get("/Installations")->body << "\n";

    std::cout << "Acquiring Manifest!\n";
    std::string manifestRoute = "/Manifest" + url;
    std::string packageYAML = client.Get(manifestRoute.c_str())->body;
    YAML::Node node = YAML::Load(packageYAML.c_str());

    std::cout << "Checking local files\n";
    std::vector<std::string> incorrectFiles;
    for (const auto& it : node) {
        std::string relativeFilePath = it.first.as<std::string>();
        std::string hash = it.second.as<std::string>();

        std::filesystem::path instalationFilePath(localInstallPath + relativeFilePath);
        if (!std::filesystem::exists(instalationFilePath))
            incorrectFiles.emplace_back(relativeFilePath);
        else if(GetFileHashForFile(instalationFilePath.string()) != hash)
            incorrectFiles.emplace_back(relativeFilePath);
    }

    if (incorrectFiles.size() == 0)
        return;

    std::string buffer;
    for (const auto& incorrectFile : incorrectFiles) {
        buffer.clear();
        client.Get(("/Files" + url + "/" + incorrectFile).c_str(),
            [&](const char* data, size_t data_length) {
                    buffer.append(data, data_length);
                    return true;
                });
        std::cout << "Patching file: " + incorrectFile + "\n";

        std::filesystem::path outputFilePath(localInstallPath + incorrectFile);

        //Create all directories needed
        std::filesystem::create_directories(outputFilePath.parent_path());

        std::fstream outputFile(outputFilePath.string(), std::ios::out | std::ios::binary | std::ios::trunc);
        outputFile.write(&buffer[0], buffer.size());
        outputFile.close();

    }

    std::cout << "Re-evaluating to make sure we're up to date \n";
    RequestInstalation(url, localInstallPath);
}

int main() {
    RequestInstalation("/TestProduct/TestFamily/Test/Windows/x64/1", InstalationDir + "\\InstallDir1");
    RequestInstalation("/TestProduct/TestFamily/Test/Linux/x84/2", InstalationDir + "\\InstallDir2");
    RequestInstalation("/TestProduct2/Family2/Dev/MacOs/ARM/1", InstalationDir + "\\InstallDir3");

	return 0;
}