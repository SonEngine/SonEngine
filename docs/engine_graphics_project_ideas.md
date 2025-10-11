# 엔진/그래픽스 직무 지원을 위한 개인 프로젝트 아이디어

아래 프로젝트들은 DirectX 12 기반 로우레벨 렌더링 엔진을 다루고 있는 현 리포지토리의 방향성과도 맞닿아 있으며, 엔진 혹은 그래픽스 프로그래머 포트폴리오에 실질적인 가치를 더해 줄 수 있는 주제들입니다. 각 아이디어마다 목표, 핵심 구현 요소, 확장 아이디어, 포트폴리오 정리 팁을 제시합니다.

## 1. 모듈형 렌더 파이프라인 리팩터링
- **목표**: 기존 렌더링 파이프라인을 모듈화하여 지연 렌더링(Deferred Shading), 포스트 프로세싱 등 다양한 패스를 유연하게 조합할 수 있는 구조 구축.
- **핵심 구현 요소**
  - Render Graph 혹은 Frame Graph 자료구조 설계 및 Pass 간 종속성 관리.
  - GPU 리소스(Light Buffer, G-Buffer 등)의 수명 관리와 Barrier 최적화.
  - 멀티스레드 커맨드 레코딩(DirectX 12 Command List 병렬화) 실험.
- **확장 아이디어**: Tiled/Clustered Lighting, Volumetric Lighting 추가.
- **포트폴리오 팁**: 설계 다이어그램과 함께 각 패스가 어떻게 연결되는지, 최적화 전후 프레임 타임 비교 그래프를 정리.

## 2. PBR 머티리얼 및 라이트 베이킹 파이프라인 구축
- **목표**: Physically Based Rendering 기반의 머티리얼 시스템을 구현하고, HDR 환경에서 라이트 베이킹(IBL, Lightmap) 파이프라인을 구성.
- **핵심 구현 요소**
  - GGX BRDF, Cook-Torrance 모델, Fresnel-Schlick 근사식 구현.
  - Image Based Lighting을 위한 Cube Map convolution, Prefilter, BRDF LUT 생성 툴 제작.
  - 라이트맵 베이킹을 위한 간단한 라이트 트레이서(RTX 또는 CPU Path Tracer) 제작.
- **확장 아이디어**: Area Light 지원, Subsurface Scattering, Transmission 효과 추가.
- **포트폴리오 팁**: 동일 씬에서 Standard vs PBR 비교 스크린샷, 베이킹 툴 UI 캡처, 베이킹 시간/퀄리티 트레이드오프 설명.

## 3. GPU 기반 애니메이션 시스템
- **목표**: Skeletal Animation 및 Morph Target을 GPU에서 처리하여 대량 캐릭터 렌더링 성능 향상.
- **핵심 구현 요소**
  - Skinning을 위한 Structured Buffer 업로드 및 Compute Shader 기반 본 행렬 계산.
  - Animation Blending State Machine 설계 및 GPU Instancing 연계.
  - LOD(Level of Detail) 시스템과 연동하여 원거리 캐릭터 단순화.
- **확장 아이디어**: Motion Matching, Animation Compression, GPU Driven Rendering(VRS, Mesh Shader)과 연동.
- **포트폴리오 팁**: CPU Skinning 대비 퍼포먼스 그래프, GPU 프로파일 캡처 분석을 첨부.

## 4. 에디터 내장 디버그/프로파일링 툴킷
- **목표**: 엔진 내부 상태를 시각화할 수 있는 디버그 뷰어 및 프로파일러 도구 개발.
- **핵심 구현 요소**
  - ImGui 등을 활용한 Render Target Inspector, Resource Viewer, GPU 타임라인 시각화.
  - Frame 캡처 및 분석(PIX, RenderDoc) 자동화 스크립트 연동.
  - 실시간 파라미터 튜닝(Exposure, Shadow Bias 등)을 위한 UI 패널 구성.
- **확장 아이디어**: 네트워크를 통한 원격 프로파일링, 에디터 스크립팅 API 제공.
- **포트폴리오 팁**: 디버그 뷰 스크린샷과 함께 문제를 어떻게 추적/해결했는지 사례 중심으로 정리.

## 5. 실시간 글로벌 일루미네이션 프로토타입
- **목표**: 화면 공간 혹은 Voxels 기반의 실시간 GI를 실험하여 현대 그래픽스 트렌드에 맞춘 기술 연구 수행.
- **핵심 구현 요소**
  - Screen Space Global Illumination(SSGI) 또는 Voxel Cone Tracing 파이프라인 구현.
  - Temporal Accumulation, Denoising 필터, History Reprojection 기법 적용.
  - 성능 최적화를 위한 Checkerboard Rendering, Half-Resolution 처리 옵션 실험.
- **확장 아이디어**: RTX Ray Tracing과 하이브리드 구성, DDGI(DDN-based Diffuse Global Illumination) 연구.
- **포트폴리오 팁**: 기법별 품질과 성능 비교 영상 제작, 알고리즘 동작 원리를 다이어그램으로 시각화.

---

### 프로젝트 운영 전략
1. **문서화**: 설계 의도, 기술적 도전 과제, 해결 과정, 성능 측정 결과를 README 혹은 기술 블로그에 체계적으로 기록합니다.
2. **코드 품질**: 모듈화, 단위 테스트(가능한 범위), 코드 리뷰(커뮤니티 참여)를 통해 유지보수성을 강조합니다.
3. **커뮤니케이션**: GitHub Issues/Project Board를 활용해 태스크 관리 과정을 공개하고, 기술 공유 세미나나 블로그 포스트로 학습 내용을 정리합니다.
4. **확장성**: 프로젝트를 단기 프로토타입으로 끝내지 말고, 장기적으로 발전시키며 릴리즈 노트와 버전 관리 히스토리를 구축합니다.

각 프로젝트는 엔진 구조 설계 능력과 최신 그래픽스 기법 이해도를 동시에 보여줄 수 있는 좋은 소재입니다. 목표 기업의 기술 스택에 맞춰 우선순위를 조정하고, 꾸준한 성능 분석과 시각 자료로 완성도를 높여 보세요.
