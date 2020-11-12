# LoRaKissTNC

430MHzのLoRaモジュールで以下の機能を実現するスケッチです。※1

1. KISSモードTNC
1. テキストチャット

「KISSモードTNC」モードでは、LoRaモジュールをKISSモードのターミナルノードコントローラとして動作させます。シリアル回線からAX25形式で送られたパケットを指定された周波数でLoRa変調により送信します。※２

「テキストチャット」モードでは、LoRaモジュールにシリアル接続された端末から送られたテキストメッセージの平文を指定された周波数でLoRa変調により送信します。

※1[w-ockham/LoRaKissTNC](https://github.com/w-ockham/LoRaKissTNC/)からフォークしており、改造版の[Arduino LoRa](https://github.com/sandeepmistry/arduino-LoRa/blob/master/README.md)の代わりに[RadioLib](https://github.com/jgromes/RadioLib/blob/master/README.md)を使用しています。

※2[APRS on LoRa](https://github.com/josefmtd/lora-aprs)のKISSモードTNCを流用しています。

このフォークで提供されるソースコードの機能および操作方法は、フォーク元の[w-ockham/LoRaKissTNC](https://github.com/w-ockham/LoRaKissTNC/)と変わりません。ただし、APRSゲートウェイ関連および変更申請についてのファイルは削除しているため、README.mdからもこれらに関する説明は削除しています。ハードウェアの記述については、こちらで動作確認を行った機材に改めています。

## 通信方式
詳細な通信方式は[プロトコル](https://github.com/jg1uaa/LoRaKissTNC/blob/jg1uaa/Protocol.md)を参照して下さい。

## ハードウェア

 * [HopeRF RFM98W](https://www.hoperf.com/modules/lora/RFM98.html)を使ったLoRaシールドとArduino UNOの組み合わせで動作します。:
   * [Dragino LoRa shield](https://www.dragino.com/products/lora/item/102-lora-shield.html)の互換品

### HopeRF RFM98W

| HopeRF RFM98W | Arduino |
| :---------------------: | :------:|
| VCC | 3.3V |
| GND | GND |
| SCK | SCK |
| MISO | MISO |
| MOSI | MOSI |
| NSS | 10 |
| NRESET | 9 |
| DIO0 | 2 |
| DIO1 | 6 |


`NSS`,`NRESET`,`DIO0`,`DIO1`に対応するピンは[Config.h](https://github.com/jg1uaa/LoRaKissTNC/blob/jg1uaa/src/LoRaKissTNC/Config.h)で定義しています。LoRaシールドの設定に応じ、変更してください。

RFM98Wの電源およびI/Oピンは3.3Vです。LoRaシールドではArduino UNOのI/Oピン(5V)に合わせるためのレベル変換を行っている点に注意してください。

## インストール方法
1. LoRaKissTNC.inoを開く  
2. スケッチ→検証・コンパイルでコンパイル  
3. スケッチ→マイコンボードに書き込むでArduinoへ書き込み  

### スマートフォンとの接続
1. スマホに[USBシリアル端末](https://play.google.com/store/apps/details?id=jp.sugnakys.usbserialconsole&hl=ja)をインストール
2. [OTGケーブル](https://www.amazon.co.jp/dp/B012V56C8K)とmicroUSBケーブルを使ってLoRa32u4をスマホに接続
3. シリアルポートの設定で  
  ボーレート9600 データビット8 パリティnone ストップビット1 フロー制御off  
  を選択
4. 接続の設定で送信フォームの表示、改行コードCR+LFを選択

## KISSモードTNCモード
### APRSクライアントの設定
APRSクライアントとしてAndroid端末上で[APRSDroid](https://aprsdroid.org/)を用います。まずAPRSDroidの設定画面でコールサインを設定して下さい。次にLoRaトランシーバをOTGケーブルで接続し、設定画面で以下を設定してください。

1. 接続方式 TNC(KISS)を選択します
2. TNC初期化設定 初期化設定文字列にTNCの初期化文字列をURLエンコードした文字を設定します。
```
%0DKISS%2043851%2C3%2C8%2C8%2C10000%0D
```
これはTNCの以下の初期化文字列をURLエンコード(`%0D=改行 %20=スペース %2C=,`)したものです。
```
 （改行)KISS 43851,3,8,8,10000(改行)
```
初期化文字列の詳細については後述します。

3. 接続タイプ USBシリアルを設定します。
4. 機器通信速度 9600bpsを設定します。

以上で完了です。

ARPSDroidを起動するとTNCへAX.25形式のパケットが送られます。LoRa対応のi-gate局からデジピートされます。

### TNC初期化文字列について
KISSコマンドにより、LoRaモジュールをKISS TNCモードにします。
初期化文字列を用いてモジュールのパラメータを以下の通り指定できます。
```
  KISS <キャリア周波数(10kHz単位)> , <帯域> , <拡散率> , <コーディングレート>, <最大バックオフ時間(ms)>
  ```
各パラメータの詳細については「テキストチャット」モードのコマンドを参照して下さい。
#### キャリア周波数
 438.0MHz～439MHzまで10kHz単位で指定します。範囲外の周波数が指定された場合は送信出来ません。
#### 帯域
以下のBW値を指定します。デフォルト値は15.6kHzです。

LoRaの電波型式はF1Dとなります。430MHz帯では総務省告示第百二十五号`無線設備規則別表第二号第54の規定に基づくアマチュア局の無線設備の占有周波数帯幅の許容値`に基づき占有帯域幅30kHz以下となりますのでご注意下さい。

| BW値 | 帯域幅 |
|:-----|:-------|
|0 | 7.8kHz |
|1 | 10.4kHz |
|2 | 15.6kHz（デフォルト値) |
|3 | 20.8kHz |

#### 拡散率
拡散率を`7(2^7=128) ～ 12(2^12=4096)`の範囲で指定できます。
#### コーディングレート
コーディングレートを `5(4/5) ～ 8(4/8)`の範囲で指定できます。
#### 最大バックオフ時間
本プログラムでは送信前に所定時間(3sec)チャンネルのアクティビティを監視し、他局が送信をしていない場合に自局からの送信を行います。  
衝突が起きた場合には3秒からここで指定された時間の範囲でランダムに待ち時間を入れます。拡散率が高い場合は衝突が起きる可能性が高いので、フレーム送出時間と同程度の長めのバックオフ時間(BW=62.5,SF=11,CR=8で10000ms程度)を設定してください。

## テキストチャットモード
テキストチャットモードでは端末から入力された文字を行単位で平文で送信します。
行頭が`set`で始まる行はコマンド列として解釈されます。

まず交信の前にコールサインを設定して下さい。（コールサインを設定しないと送信出来ません）
```sh
 set call <あなたのコールサイン>
```

次にコマンドを使って運用周波数やLoRa変調のパラメータを設定して下さい。デフォルトでは周波数438.51MHz、SF=8、BW=15.6kHz 出力20dBm(100mW)の設定になっています。
```sh
set
Freq=43851
SF=8
BW=2　(15.6kHz)
TXpower=20
```
ターミナルからメッセージを入れ、最後に改行(CR/LF)を入力して下さい。
メッセージの送信が完了すると以下のように表示されます。
```sh
 <コールサイン> >: <送信したメッセージ>
```
相手局からメッセージを受信すると以下のように表示されます。
```sh
 <相手局コールサイン> (<RSSI値>,<SNR値>,<周波数エラー値>)<: <受信したメッセージ>
 ```

## コマンド

### 自局コールサインの指定
自局のコールサインを指定します。パケット先頭には必ず自局コールサインが入ります。
またコールサインが指定されていない場合送信できません。
```sh
set call コールサイン
```

### 周波数の設定
運用周波数を10kHz単位で指定します。
指定できる範囲は438MHz-439MHz(全電波形式の範囲)です。
デフォルト値は438.51MHzです。
```sh
 set freq 43851
```
### 送信出力の設定
送信出力をdBmで指定します。2dBm - 20dBmの範囲です。
デフォルト値は20dBmです。
```sh
 set pwr 20
```
### 拡散率(SF)の指定
拡散率(Spreading Factor)を指定します。6 - 12の範囲です。
  小さい値ほど高速に送信できますがSNRでは不利になります。
  デフォルト値は9です。
```sh
 set sf 9
```
### 帯域幅(BW)の指定
チャープスペクトラムの帯域幅を指定します。0-8の範囲です。帯域幅が広いほど高速に送信できますがSNRでは不利になります。

```sh
 set bw 2
 ```
| BW値 | 帯域幅 |
|:-----|:-------|
|0 | 7.8kHz |
|1 | 10.4kHz |
|2 | 15.6kHz (デフォルト値) |
|3 | 20.8kHz |

### 設定の確認
`set`コマンドでパラメータを指定しないと現在の設定値が表示されます。
```sh
Freq=43851
SF=9
BW=2　(15.6kHz)
TXpower=20
```

### 設定の初期化
設定をデフォルト値に戻します。
```sh
set init
```
