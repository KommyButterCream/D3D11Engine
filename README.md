# D3D11Engine

## 프로젝트 설명

Direct3D 11 렌더링 엔진 DLL이다. 장치와 렌더링 컨텍스트를 관리하며 Direct2D·DirectWrite 연동, 2D 카메라, 폰트 관리를 제공한다.

## 빌드 환경

- Windows 10 또는 11 x64 및 Windows SDK
- Visual Studio 2026의 **C++를 사용한 데스크톱 개발** 워크로드와 MSVC v145
- C++20

## 저장소 배치와 빌드

다음 폴더를 같은 상위 폴더에 배치한다.

```text
Module/
├─ Core/
├─ D3D11EngineInterface/
└─ D3D11Engine/
```

Visual Studio 2026에서 `D3D11Engine.sln`을 열고 `Debug|x64` 또는 `Release|x64`로 빌드한다. 결과 DLL은 솔루션의 `x64/Debug/` 또는 `x64/Release/`에 생성된다.

빌드한 솔루션의 `Shaders/`에 HLSL 컴파일 결과(`.cso`)가 생성된다.
