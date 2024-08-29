# UE_LyraSplatoon
언리얼 엔진에서 제공하는 샘플 프로젝트인 [Lyra Starter game](https://www.unrealengine.com/marketplace/en-US/product/lyra)을 확장 개발한 게임입니다.  
닌텐도 플랫폼에서 발매된 게임인 `Splatoon`시리즈의 독특한 기능들을 라이라의 코드베이스를 기반으로 구현했습니다.
  
    
본 프로젝트는 Lyra Starter Game의 GameFeatures Plugin형태로 제작되었으므로  
Lyra Core 소스코드의 수정 없이 플러그인 설치 형태로 적용 가능합니다.  
Lyra Core일부 클래스는 `LYRAGAME_API` 매크로가 사용되지 않아서 상속+추가구현이 불가능했기에 수정이 불가피 했고 해당 수정사항은 patch파일로 제공합니다.(하단 문서 참고)

# Install

- UE 5.4.2버전을 설치합니다.(추후 업데이트되는 언리얼 엔진에도 대응될 것으로 예상되나 본 플러그인은 5.4.2버전에서 개발되었기에 5.4.2버전을 추천합니다)
- [Lyra Starter Game](https://www.unrealengine.com/marketplace/en-US/product/lyra) 을 설치합니다.
- `LyraStarterGame/Plugins/GameFeatures/` 경로에 본 프로젝트 파일들을 다운받아 넣어줍니다.
    - 방법1. zip파일로 다운 + 압축해제
    - 방법2. `git clone https://github.com/yuntaewoong/UE_LyraSplatoon.git ANYNAME` 명령어로 프로젝트 클론
- 컴퓨트 셰이더기능을 위해 플러그인 형태로 별도로 구현한 [Compute Shader Plugin](https://github.com/yuntaewoong/UE5_ComputeShaderPlugin)을 다운 받은 후에 `ComputeShader_Plugin` 폴더를 `LyraStarterGame/Plugins/` 경로에 넣어줍니다.
- `Init.sh` 쉘 스크립트를 실행해서 `patch.txt`의 패치파일을 라이라 c++ 코드에 적용시킵니다.  
(주의:쉘스크립트 실행 시 `LyraStarterGame/Source/LyraGame`에 존재하는 일부 c++ 헤더파일에 변경사항이 적용됩니다)
- `LyraStarterGame.uproject` 파일을 실행시킵니다.
- 언리얼 에디터에서 플러그인 창으로 들어가서 `Lyra Splatoon`,`ComputeShaderPlugin` 플러그인을 검색 후 활성화 합니다.

- 언리얼 에디터를 종료 후 다시 실행합니다.
- `콘텐츠 브라우저에서 Plugins/LyraSplatoon Contents/Maps/Main.umap` 맵을 에디터에서 열고 실행합니다


# Main Features

- 지형, 물건에 페인트 효과 적용
- 게임 규칙 적용(더 많은 영역을 색칠한 팀의 승리처리)
- 위 기능들이 네트워크 상에서 완벽히 동작하는 것