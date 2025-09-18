#pragma once

#include <unordered_map>
#include <directxtk12\SimpleMath.h>

namespace Input {
	class InputHelper {
	public:
		InputHelper();

		void SetInputState(size_t wParam, bool bKeyDown);
		bool GetInputState(size_t wParam);
		DirectX::SimpleMath::Vector3 ExecuteCommands(const float& deltaTime, const DirectX::SimpleMath::Vector3& eyeDirection, const  DirectX::SimpleMath::Vector3& upDirection, const  DirectX::SimpleMath::Vector3& rightDirection);
	private:
		std::unordered_map<size_t, bool> InputStates;

	public:
		size_t upKey = size_t('E');
		size_t downKey = size_t('Q');
		size_t rightKey = size_t('D');
		size_t leftKey = size_t('A');
		size_t forwardKey = size_t('W');
		size_t backwardKey = size_t('S');

	private:
		float cameraSpeed  = 1.f;
	};
};
