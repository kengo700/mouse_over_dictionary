# Mouse Over Dictionary

マウスオーバーした単語を自動で読み取る汎用辞書ツールです。Windows用のオープンソースソフトウェアです。

**開発初期段階のアルファ版です。自己責任でご利用ください。不具合報告や改善案をいただけると助かります！**

![DOS2](https://github.com/kengo700/mouse_over_dictionary/blob/images/mod_dos2.png)
（CRPG[『Divinity: Original Sin 2』](https://store.steampowered.com/app/435150/Divinity_Original_Sin_2__Definitive_Edition/)で使用した様子）

## 特徴

* マウスオーバーするだけで自動的に検索
* あらゆる場面で使える（ゲーム、PDF、Excel、Kindleなど）
* 好きな辞書データをインポート可能

## 使い方

### インストール
* 下記からダウンロードして解凍
    * [MouseOverDictionary.ver.0.0.3.zip](https://github.com/kengo700/mouse_over_dictionary/releases/download/v0.0.3/MouseOverDictionary.ver.0.0.3.zip)

### 主な機能

* マウスオーバー辞書引き
    * 調べたい単語にマウスカーソルを近づけると、自動的に読み取って辞書を引きます
* マウス追従ウィンドウ
    * 検索欄の右のアイコンで、マウスに追従するウィンドウを表示します
* 検索履歴
    * 検索欄の左のアイコンで、これまで検索してヒットした単語一覧を表示します

### 辞書データ

下記の辞書データに対応しています。dictionaryフォルダに入れておくと起動時に読み込みます
* [ejdic-handテキスト形式（UTF-8）](https://github.com/kujirahand/EJDict)
    * パブリックドメイン、6万5600項目以上
    * 本ソフトに同梱
* [英辞郎テキスト形式（Shift-JIS）](https://booth.pm/ja/items/777563)
    * ¥495、198万9500項目
    * おすすめ
* [PDIC１行テキスト形式（Unicode、Shift-JIS）](http://pdic.la.coocan.jp/unicode/help/OneLineFormat.html)
   * PDICで変換可能

## その他

### 動作環境

* Windows 10 64bit
* 高DPI非対応

### ライセンス

* [The MIT License (MIT)](LICENSE.txt)

### ライブラリ

* [Tesseract](https://github.com/tesseract-ocr/tesseract)
    * 用途：文字認識
    * ライセンス：Apache License 2.0
    * ライセンス全文：[licenses/LICENSE_Tesseract.txt](licenses/LICENSE_Tesseract.txt)
* [Qt](https://www.qt.io)
    * 用途：インターフェース構築
    * ライセンス：GNU General Public License v3.0
    * ライセンス全文：[licenses/LICENSE_Qt.txt](licenses/LICENSE_Qt.txt)

### リファレンス

* プログラミング
    * [Stack Overflow - How to make tesseract-ocr read from coordinates on a screen?](https://stackoverflow.com/questions/22924209)
    * [GitHub - wtetsu - mouse-dictionary](https://github.com/wtetsu/mouse-dictionary)
    * [GitHub - mrenouf - Plurals.java](https://gist.github.com/mrenouf/805745)
* 辞書形式
    * [GitHub - kujirahand - EJDict](https://github.com/kujirahand/EJDict)
    * [EDP - 英辞郎とは - データ仕様](http://www.eijiro.jp/spec.htm)
    * [PDIC/Unicode - １行テキスト形式](http://pdic.la.coocan.jp/unicode/help/)
* アイコン
    * [REMIX ICON](https://remixicon.com/)
