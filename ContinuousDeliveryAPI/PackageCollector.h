#pragma once

#include <string>
#include <unordered_map>

namespace ContinuousDelivery {

	/// <summary>
	/// This class is used to convert a folder into a sendable package
	/// </summary>
	class PackageCollector {

		struct PackageLine {
			std::string Path;
			std::string Hash;
		};

	public:
		PackageCollector(std::string folderPath);

		std::string Assemble();

	private:

		void FillPackage(std::vector<PackageLine>& contents);
		std::string PackYAML(std::vector<PackageLine>& contents);

		std::string _folder;
	};
}