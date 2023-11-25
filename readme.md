# LVGL 用 ILI9341 汎用ドライバ

LVGL  https://github.com/lvgl/lvgl の Raspberry Pi Pico 用 ILI9341 汎用ドライバです。

## 引用したもの

- lvgl Light and Versatile Graphics Library

プロジェクトのディレクトリに git clone します。
- spi.pio

pico-example の中にある software spi のサンプル PIO ファイルです。

## 構築手順

- このリポジトリを git clone する
- cd リポジトリ
- git clone https://github.com/lvgl/lvgl.git
- cd lvgl
- cp lv_conf_template.h lv_conf.h
- テキストエディタで lv_conf.h を修正する

`#if 0 /*Set it to "1" to enable content*/`

#if 0 を #if 1 に修正して lv_conf.h 内の define を有効にする

`#define LV_USE_DEV_VERSION`

を追記する

`#define LV_COLOR_DEPTH 16`

`#define LV_MEM_SIZE (48 * 1024U)          /*[bytes]*/`


を確認する

- `cd ..`
- `mkdir build`
- `cd build`
- `cmake -DCMAKE_BUILD_TYPE=Debug  -DPICO_DEOPTIMIZED_DEBUG=on ..`

MSYS2 Mingw64 でビルドするときは -G "MSYS Makefiles" を追記する

- `make`

です。

lvgl ライブラリを add_subdirectory しているだけの環境なので難しくはないと思います。

2023/11/12 追記

8cf0bbb - feat(draw): add implements vector graphic APIs (#4528) (#4691)

で、機能追加がありましたが、エラーになります。CmakeLists.txt でこれをコンパイルしないよう Disable に暫定対応しています。

```
[ 47%] Building CXX object lvgl/CMakeFiles/lvgl_thorvg.dir/src/libs/thorvg/tvgAnimation.cpp.obj
In file included from C:/msys64/home/masahiro/work/ILI9341_lvgl_driver_for_RP2040/lvgl/src/libs/thorvg/tvgAnimation.cpp:23:
c:\msys64\home\masahiro\work\ili9341_lvgl_driver_for_rp2040\lvgl\src\lv_conf_internal.h:41:18: fatal error: ../../lv_conf.h: No such file or directory
   41 |         #include "../../lv_conf.h"                /*Else assume lv_conf.h is next to the lvgl folder*/
      |                  ^~~~~~~~~~~~~~~~~
compilation terminated.
make[2]: *** [lvgl/CMakeFiles/lvgl_thorvg.dir/build.make:76: lvgl/CMakeFiles/lvgl_thorvg.dir/src/libs/thorvg/tvgAnimation.cpp.obj] Error 1
make[1]: *** [CMakeFiles/Makefile2:1770: lvgl/CMakeFiles/lvgl_thorvg.dir/all] Error 2
make: *** [Makefile:136: all] Error 2

```

