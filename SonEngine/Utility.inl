#pragma once

#include "StaticMeshComponent.h"
#include "PointCloudComponent.h"
#include "DotComponent.h"
#include "CubeMapComponent.h"
#include "StaticMesh.h"
#include "d3d12.h"
#include "Utility.h"
#include "World.h"

#include <sstream>

namespace GraphicsUtils {

	template<typename Data>
	inline void Utility::CreateTextureBuffer(Data* data, UINT bytesPerData, UINT width, UINT height, D3D12_RESOURCE_FLAGS Flag, Microsoft::WRL::ComPtr<ID3D12Resource>& gpuBuffer, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
	{
		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = Flag;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(gpuBuffer.ReleaseAndGetAddressOf())
		));

		UINT64 uploadSize = 0;
		m_device->GetCopyableFootprints(&texDesc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadSize);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.ReleaseAndGetAddressOf())
		));
		D3D12_SUBRESOURCE_DATA subresource = {};
		subresource.pData = data;
		subresource.RowPitch = width * bytesPerData;
		subresource.SlicePitch = subresource.RowPitch * height;

		UpdateSubresources(
			m_commandList,
			gpuBuffer.Get(),
			uploadBuffer.Get(),
			0, 0, 1, &subresource);

		//auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		//	gpuBuffer.Get(),
		//	D3D12_RESOURCE_STATE_COPY_DEST,
		//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		//m_commandList->ResourceBarrier(1, &barrier);
	}
	inline void Utility::CreateTextureBuffer(
		Microsoft::WRL::ComPtr<ID3D12Resource>& gpuBuffer,
		UINT width,
		UINT height,
		DXGI_FORMAT format,
		D3D12_RESOURCE_FLAGS Flag,
		D3D12_RESOURCE_STATES state,
		UINT16 mipLevels

	)
	{
		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = mipLevels;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = Flag;

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = format;

		clearValue.Color[0] = 0.f;
		clearValue.Color[1] = 0.f;
		clearValue.Color[2] = 0.f;
		clearValue.Color[3] = 1.f;
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			state,
			&clearValue,
			IID_PPV_ARGS(gpuBuffer.ReleaseAndGetAddressOf())
		));

	}
	inline void Utility::CreateResourceView(Microsoft::WRL::ComPtr<ID3D12Resource>& buffer, DXGI_FORMAT format, bool bUseMsaa, D3D12_CPU_DESCRIPTOR_HANDLE& handle, const DescriptorType& type)
	{
		if (type == DescriptorType::RTV) {
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
			ZeroMemory(&rtvDesc, sizeof(rtvDesc));
			if (bUseMsaa) {
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
			}
			else {
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			}
			rtvDesc.Format = format;
			rtvDesc.Texture2D.MipSlice = 0;

			m_device->CreateRenderTargetView(buffer.Get(), &rtvDesc, handle);
		}
		else if (type == DescriptorType::UAV) {
			if (bUseMsaa) {
				std::cout << "UAV는 MSAA로 만들 수 없습니다" << std::endl;
				return;
			}
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			ZeroMemory(&uavDesc, sizeof(uavDesc));
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Format = format;
			uavDesc.Texture2D.MipSlice = 0;
			m_device->CreateUnorderedAccessView(buffer.Get(), nullptr, &uavDesc, handle);
		}
		else if (type == DescriptorType::SRV) {

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			if (bUseMsaa) {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			}
			else {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			}
			srvDesc.Format = format;
			srvDesc.Texture2D.MipLevels = 1;
			m_device->CreateShaderResourceView(buffer.Get(), &srvDesc, handle);
		}
	}

	inline std::string Utility::MakeTimestamp()
	{
		using namespace std::chrono;

		const auto now = system_clock::now();
		const std::time_t t = system_clock::to_time_t(now);

		std::tm tm{};
#if defined(_WIN32)
		localtime_s(&tm, &t);
#else
		localtime_r(&t, &tm);
#endif

		std::ostringstream oss;
		oss << std::put_time(&tm, "_%y%m%d_%H%M%S");
		return oss.str();
	}

	template<typename Data>
	inline void Utility::CreateBuffer(
		const std::vector<Data>& data,
		Microsoft::WRL::ComPtr<ID3D12Resource>& gpuBuffer,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
	{
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(Data) * data.size()),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf())
		));

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(Data) * data.size()),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(gpuBuffer.GetAddressOf())
		));

		D3D12_SUBRESOURCE_DATA subData;
		subData.pData = data.data();
		subData.RowPitch = sizeof(Data) * data.size();
		subData.SlicePitch = subData.RowPitch;

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpuBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

		UpdateSubresources(
			m_commandList,
			gpuBuffer.Get(),
			uploadBuffer.Get(),
			0, 0, 1, &subData
		);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpuBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));

	}
	template<typename V, typename I, typename MeshType>
	inline std::shared_ptr<Actor> Utility::CreateActor(
		const std::string& actorname,
		const std::vector<Mesh<V, I>>& meshes,
		const std::string& texture,
		const DirectX::SimpleMath::Vector3& location,
		World* world,
		bool simulate,
		PhysXMode physXMode, 
		int foreceMip0
	)
	{
		std::shared_ptr<Actor> actor = std::make_shared<Actor>(actorname, world);

		std::shared_ptr<MeshType> mesh = std::make_shared<MeshType>();
		if (StaticMesh* m = dynamic_cast<StaticMesh*>(mesh.get()))
		{
			m->Initialize(m_device, m_commandList, meshes);
			m->SetAlbedoTexture(texture);
			//mesh->SetLocation(location.x, location.y, location.z);
			//
			std::shared_ptr<StaticMeshComponent> cmp = std::make_shared<StaticMeshComponent>(actor.get());
			cmp->SetMesh(mesh);
			cmp->SetPhysX(simulate);
			cmp->SetPhysXMode(physXMode);
			actor->SetRootComponent(cmp);
		}
		actor->SetActorLocation(location);
		actor->UpdateMipState(foreceMip0);
		return actor;
	}
	template<typename V, typename I, typename MeshType, typename RootComponentType>
	inline std::shared_ptr<Actor> Utility::CreateActor2(
		const std::string& actorname,
		const std::vector<Mesh<V, I>>& meshes,
		const std::string& texture,
		const DirectX::SimpleMath::Vector3& location,
		World* world,
		bool simulate,
		PhysXMode physXMode,
		int foreceMip0
	)
	{
		std::shared_ptr<Actor> actor = std::make_shared<Actor>(actorname, world);

		std::shared_ptr<MeshType> mesh = std::make_shared<MeshType>();
		if (StaticMesh* m = dynamic_cast<StaticMesh*>(mesh.get()))
		{
			if constexpr (std::is_same_v<RootComponentType, StaticMeshComponent> ||
				std::is_same_v<RootComponentType, CubeMapComponent>)
				m->Initialize(m_device, m_commandList, meshes);
			else
				m->InitializePC(m_device, m_commandList, meshes);
			m->SetAlbedoTexture(texture);

			std::shared_ptr<RootComponentType> cmp = std::make_shared<RootComponentType>(actor.get());
			cmp->SetMesh(mesh);
			cmp->SetPhysX(simulate);
			cmp->SetPhysXMode(physXMode);
			actor->SetRootComponent(cmp);
		}
		actor->UpdateMipState(foreceMip0);
		actor->SetActorLocation(location);
		return actor;
	}

	template<typename V, typename I, typename MeshType>
	inline std::shared_ptr<Actor> Utility::CreatePCActor(
		const std::string& actorname,
		const std::vector<Mesh<V, I>>& meshes,
		const std::string& texture,
		World* world,
		bool simulate,
		PhysXMode physXMode,
		int foreceMip0
	)
	{
		std::shared_ptr<Actor> actor = std::make_shared<Actor>(actorname, world);

		std::shared_ptr<MeshType> mesh = std::make_shared<MeshType>();
		if (StaticMesh* m = dynamic_cast<StaticMesh*>(mesh.get()))
		{
			m->InitializePC(m_device, m_commandList, meshes);
			m->SetAlbedoTexture(texture);
			std::shared_ptr<PointCloudComponent> cmp = std::make_shared<PointCloudComponent>(actor.get());
			cmp->SetMesh(mesh);
			cmp->SetPhysX(simulate);
			cmp->SetPhysXMode(physXMode);
			actor->SetRootComponent(cmp);
		}
		actor->UpdateMipState(foreceMip0);

		return actor;
	}

	template<typename A, typename V, typename I, typename MeshType>
	inline std::shared_ptr<A> Utility::CreateActor(
		const std::string& actorname,
		const std::vector<Mesh<V, I>>& meshes,
		const std::string& texture,
		const DirectX::SimpleMath::Vector3& location,
		World* world,
		bool simulate,
		PhysXMode physXMode,
		int foreceMip0
	)
	{
		std::shared_ptr<A> actor = std::make_shared<A>(actorname, world);

		std::shared_ptr<MeshType> mesh = std::make_shared<MeshType>();
		if (StaticMesh* m = dynamic_cast<StaticMesh*>(mesh.get()))
		{
			m->Initialize(m_device, m_commandList, meshes);
			m->SetAlbedoTexture(texture);

			std::shared_ptr<StaticMeshComponent> cmp = std::make_shared<StaticMeshComponent>(actor.get());
			cmp->SetMesh(mesh);
			cmp->SetPhysX(simulate);
			cmp->SetPhysXMode(physXMode);
			actor->SetRootComponent(cmp);
		}
		actor->UpdateMipState(foreceMip0);
		actor->SetActorLocation(location);
		return actor;
	}


}