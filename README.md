cvtest
======

A trivial sample program using IplImage of OpenCV for an OpenGL texture

これは OpenCV の画像 (IplImage) を OpenGL のテクスチャとして使うサンプルサンプルです。
カメラからキャプチャした画像を、回転する球の表面にマッピングします。
そのままマッピングしたのでは面白くないので、
グレイレベルを微分したものを法線ベクトルに使って、バンプマッピングを行っています。
カメラからの画像の取得は OpenGL (GLUT) の描画ループとは別スレッドにしています。
