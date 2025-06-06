#pragma once

#include "BaseApp.h"

namespace Core {
	class SimpleApp : public BaseApp
	{

	public:
		SimpleApp();
		SimpleApp(const int width, const int height);

	private:

		// Init

		virtual bool InitDirectX() override;
		virtual bool InitGUI() override;
		
		// Called when the window is resized
		virtual void OnResize() override;

		virtual void Update(float deltaTime) override;
		virtual void UpdateGUI(float deltaTime) override;

		virtual void Render(float deltaTime) override;
		virtual void RenderGUI(float deltaTime) override;

		// Fin

		virtual bool FinDirectX() override;
		virtual void Finalize(float deltaTime) override;

	};
}