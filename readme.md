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

を確認する

- `cd ..`
- `mkdir build`
- `cd build`
- `cmake -DCMAKE_BUILD_TYPE=Debug  -DPICO_DEOPTIMIZED_DEBUG=on ..`

MSYS2 Mingw64 でビルドするときは -G "MSYS Makefiles" を追記する

- `make`

です。

lvgl ライブラリを add_subdirectory しているだけの環境なので難しくはないと思います。
