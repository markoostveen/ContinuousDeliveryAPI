#include "PackageCollector.h"

#include "FileHashes.h"

#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <iostream>
#include <sstream>

namespace ContinuousDelivery {

	PackageCollector::PackageCollector(std::string folderPath)
		: _folder(folderPath)
	{
	}

	std::string PackageCollector::Assemble()
	{
		std::vector<PackageLine> contents;
		
		FillPackage(contents);
		return PackYAML(contents);
	}

	void PackageCollector::FillPackage(std::vector<PackageLine>& contents)
	{
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(_folder)) {
			if (dirEntry.is_directory())
				continue; // do not include directories

			std::string fullPath = dirEntry.path().string();
			std::string relativePath = fullPath.substr(_folder.size(), fullPath.size() - _folder.size());

			std::string hash = GetFileHashForFile(fullPath);
			if (hash == "")
				return;

			PackageLine Line;
			Line.Hash = hash;
			Line.Path = relativePath;

			contents.emplace_back(Line);

			//std::cout << relativePath << std::endl;
		}
	}

	std::string PackageCollector::PackYAML(std::vector<PackageLine>& contents)
	{
		int packIndex = 0;
		int packNumber = 0;
		YAML::Emitter emitter;
		emitter << YAML::BeginMap;
		for (int i = contents.size(); i > 0; i -= 50)
		{
			packNumber++;

			int packSize = 50;
			if (49 - i > 0)
				packSize = i;

			for (int j = 0; j < packSize; j++)
			{
				emitter << YAML::Key << contents[packIndex].Path;
				emitter << YAML::Value << contents[packIndex].Hash;

				packIndex++;
			}
		}
		emitter << YAML::EndMap;

		return std::string(emitter.c_str());
	}

}