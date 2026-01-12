#include "pch.h"
//#include "DLModel.h"
//
//namespace fs = std::filesystem;
//
//DLModel::DLModel() 
//	:device(std::make_unique<c10::Device>(c10::Device("cpu")))
//{
//	
//}
//
//DLModel::~DLModel() = default;
//
//void DLModel::Initialize(const std::string& newPath)
//{
//	modelPath = fs::absolute(newPath);
//
//}
//
//int DLModel::Run(const std::vector<uint8_t>& image)
//{
//	std::ifstream in(modelPath, std::ios::binary);
//	if (!in) {
//		std::cerr << "Failed to open model file: " << modelPath.string() << "\n";
//		return -1;
//	}
//	torch::jit::script::Module module = torch::jit::load(in);
//	module.eval();
//	module.to(*device.get());
//
//	//std::cout << "image size : "<<  image.size() << '\n'; // 28 * 28 * 4
//
//	int w = 28;
//	int h = 28;
//	int c = 4;
//	std::vector<float> arr(w*h);
//	for (int y = 0; y < h; y++)
//	{
//		for (int x = 0; x < w; x++)
//		{
//			int idx_arr = x + h * y;
//			int idx_image = (x * c) + (h * y * c);
//			arr[idx_arr] = image[idx_image] / 255.f;
//		}
//	}
//	auto x = torch::from_blob(
//		arr.data(),
//		{ 1, 28 * 28 },
//		torch::TensorOptions().dtype(torch::kFloat32)
//	).clone();
//
//	x.to(*device.get());
//	//std::cout << "x shape = " << x.sizes() << "\n";
//
//	torch::NoGradGuard no_grad;
//	torch::Tensor logits = module.forward({ x }).toTensor();
//	torch::Tensor probs = torch::softmax(logits, /*dim=*/1);
//	torch::Tensor pred = probs.argmax(/*dim=*/1);
//
//	return pred.item<int64_t>();
//}