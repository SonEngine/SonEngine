//#pragma once
//
//#include <filesystem>
//#include <fstream>
//#include <memory>
//
//namespace c10 { struct Device; }
//
//class DLModel {
//public:
//	DLModel();
//	~DLModel();
//
//	void Initialize(const std::string& newPath);
//
//	int Run(const std::vector<uint8_t>& image);
//
//private:
//	std::unique_ptr<c10::Device> device;
//	std::filesystem::path modelPath;
//
//};