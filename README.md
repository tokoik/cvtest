# cvtest - OpenCV 画像を OpenGL テクスチャとして用いるバンプマッピング

## 1. 概要

本プログラムは、OpenCV でカメラからキャプチャした映像をテクスチャとして OpenGL の回転する球表面にマッピングし、輝度勾配（微分値）をもとに法線マップを生成してバンプマッピングを行うサンプルプログラムです。

カメラからの画像キャプチャは OpenGL (GLUT) の描画ループとは独立したワーカースレッドで非同期に行われます。

- 移行元ブログ記事:
  - [画像処理とグラフィックスの連携 (cvtest) - 床井研究室](https://tokoik.github.io/blog/2011-08-11.html)

## 2. 対応環境

- **Windows**: Visual Studio 2019 / 2022 / 2026 (CMake 経由で freeglut および OpenCV 4.13.0 を自動構成)
- **macOS**: Xcode (macOS 標準の GLUT および OpenCV を使用)
- **Ubuntu Linux**: GCC / Make (システム標準の freeglut3-dev, libopencv-dev を使用)

## 3. ビルド手順

### Windows (Visual Studio)

```pwsh
cmake -B build -S .
cmake --build build --config Release
```

### macOS (Xcode)

```bash
brew install opencv
cmake -B build -G Xcode
cmake --build build --config Release
```

### Ubuntu Linux (Makefile)

```bash
sudo apt-get update
sudo apt-get install -y freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev libopencv-dev
cmake -B build -S .
cmake --build build
```

## 4. 起動方法

ビルド完了後、生成された実行ファイルを実行します。

- **Windows**: `build/Release/cvtest.exe`
- **macOS**: `build/Release/cvtest.app`
- **Linux**: `build/cvtest`

## 5. 操作方法

- **[q] / [Q] / [ESC]**: プログラムの終了

## 6. プログラムの解説

OpenCV の `cv::VideoCapture` により別スレッドでカメラフレームを取得し、OpenGL の `glTexSubImage2D` でテクスチャへ転送します。シェーダ側（`simple.frag`）でテクスチャサンプルの近傍差分から法線ベクトルを動的に計算し、球表面に凹凸のある陰影（バンプマッピング）を生成しています。
