# 🎮 3D Graphics Engine Project

## 🧾 프로젝트 개요
DirectX 12을 사용한 로우레벨 3D 렌더링 엔진.  
렌더링 파이프라인 구성, 셰이더 처리, 모델 로딩 등을 직접 구현함.

## 🔧 주요 기능

### 1. Point Cloud
<img src="SonEngine/Images/BackBuffer_251214_163908.png" width="320" height="180"> <img src="SonEngine/Images/P1180209.JPG" width="320" height="180">


## 💻 Windows
### Installing Dependencies
```
vcpkg install directxtex[core,dx11,openexr]:x64-windows
vcpkg install directxtk12[core,xaudio2-9]:x64-windows
vcpkg install directxtk[core,xaudio2-9]:x64-windows
vcpkg install directx-headers:x64-windows
vcpkg install fp16:x64-windows glm:x64-windows
vcpkg install imgui[core,dx11-binding,dx12-binding,win32-binding]:x64-windows
vcpkg install assimp:x64-windows
vcpkg install boost-serialization:x64-windows
vcpkg install physx:x64-windows
vcpkg install physx:x64-windows
```
