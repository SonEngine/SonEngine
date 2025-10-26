#pragma once

#include "StaticMeshComponent.h"
#include "StaticMesh.h"

namespace GraphicsUtils {

	template<typename Data>
	inline void Utility::CreateTextureBuffer(Data* data, UINT bytesPerData, UINT width, UINT height, Microsoft::WRL::ComPtr<ID3D12Resource>& gpuBuffer, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
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
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

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

	template<typename V, typename I>
	inline std::shared_ptr<Actor> Utility::CreateActor(const std::string& actorname, Mesh<V, I>& meshData, const std::string& texture, const DirectX::SimpleMath::Vector3& location)
	{
		std::shared_ptr<Actor> actor = std::make_shared<Actor>(actorname);

		std::shared_ptr<StaticMesh> mesh = std::make_shared<StaticMesh>();
		mesh->Initialize(m_device, m_commandList, meshData);
		mesh->SetAlbedoTexture(texture);
		mesh->SetLocation(location.x, location.y, location.z);

		std::shared_ptr<StaticMeshComponent> cmp = std::make_shared<StaticMeshComponent>(actor.get());
		cmp->SetMesh(mesh);

		actor->SetRootComponent(cmp);

		return actor;
	}
}