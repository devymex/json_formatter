#include <fstream>
#include <iomanip>
#include <iostream>
#include <glog/logging.h>
#include "json.hpp"

bool LoadFileContent(const std::string &strFn, std::string &strFileBuf) {
	std::ifstream inFile(strFn, std::ios::binary);
	if (!inFile.is_open()) {
		return false;
	}
	inFile.seekg(0, std::ios::end);
	strFileBuf.resize((uint64_t)inFile.tellg());
	inFile.seekg(0, std::ios::beg);
	inFile.read(const_cast<char*>(strFileBuf.data()), strFileBuf.size());
	CHECK(inFile.good());
	return true;
}

std::pair<bool, nlohmann::json> ParseJson(std::string strText) {
	auto ret = std::make_pair(false, nlohmann::json());
	try {
		ret.second = nlohmann::json::parse(strText);
		ret.first = true;
	} catch (...) {
	}
	return ret;
}

std::pair<bool, std::vector<nlohmann::json>> ParseJsonLines(std::string strText) {
	auto ret = std::make_pair(true, std::vector<nlohmann::json>());
	std::istringstream iss(strText);
	for (std::string strLine; std::getline(iss, strLine); ) {
		try {
			ret.second.push_back(nlohmann::json::parse(strLine));
		} catch (...) {
			ret.second.clear();
			return ret;
		}
	}
	return ret;
}

int main(int nArgCnt, char *ppArgs[]) {
	FLAGS_alsologtostderr = 1;
	google::InitGoogleLogging(ppArgs[0]);

	if (nArgCnt < 3) {
		std::cout << "Usage: " << ppArgs[0] << std::endl;
		return -1;
	}

	std::string strText;
	CHECK(LoadFileContent(ppArgs[1], strText)) << ppArgs[1];
	
	std::string strOutPath = ppArgs[2];
	CHECK(!strOutPath.empty());
	int nr = 0;
	if (strOutPath.back() == '/') {
		auto [bSuc, jsons] = ParseJsonLines(strText);
		if (bSuc) {
			for (uint32_t i = 0; i < jsons.size(); ++i) {
				std::ostringstream ossFilename;
				ossFilename << std::setfill('0');
				ossFilename << strOutPath << std::setw(4) << i << ".json";
				std::ofstream outFile(ossFilename.str());
				CHECK(outFile.is_open());
				outFile << jsons[i].dump(4);
			}
			std::cout << "Write " << jsons.size() << " jsons to \""
					  << strOutPath << "\"" << std::endl;
		} else {
			std::cerr << "Specified output is a path, "
					  << "but the input is not a jsonlines!" << std::endl;
			nr = -1;
		}
	} else {
		std::ofstream outFile(strOutPath);
		CHECK(outFile.is_open());
		auto [bSuc, json] = ParseJson(std::move(strText));
		if (bSuc) {
			outFile << json.dump(4);
			std::cout << "Write to \"" << strOutPath << "\"" << std::endl;
		} else {
			std::cerr << "Specified output is a file, "
					  << "but the input is not a json!" << std::endl;
			nr = -1;
		}
	}
	return nr;
}

