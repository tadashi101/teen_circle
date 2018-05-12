# 組み立て説明書  
## 内容物  
  
| 名称 | 必要数 | 備考 |
|-----------:|-----------:|-----------:|
| LED基板 | 1 |  |
| 電源基板 | 1 | 実装済み |
| LED赤 | 240 | 少し多めに入れてます |
| LED青 | 120 | 少し多めに入れてます |
| LEDドライバ MBI5024 | 8 |  |
| ESP-WROOM-32 | 1 |  |
| 抵抗10k 1608サイズ | 11 | 1個余分に入ってるかも |
| コンデンサ10u 1608サイズ | 1 |  |
| コンデンサ0.1u 1608サイズ | 10 | 1個余分に入ってるかも |
| ピンヘッダオス 2×3 1.27mmピッチ | 1 |  |
| ネジ | 1 |  |
| スペーサー | 1 |  |
| Tバー | 1 |  |
| ワッシャー | 1 |  |
| ACアダプタ9V1.3A | 1 |  |  
  
半田付け後の基板はこのようになります  
<img src="../image/image_buildguide_1.jpg" width=70% title="半田付け済みLED基板表">
<img src="../image/image_buildguide_2.jpg" width=70% title="半田付け済みLED基板裏">
## LEDの半田付け  
LED赤とLED青を半田付けしていきます 
<img src="../image/image_buildguide_3.jpg" width=70% title="LED赤と青">  
LED赤は240個、LED青は120個実装します。量が多い方がLED赤ですね    
確認のためテスターで光らせて色を見てください  
4列縦に並んでいるのがLED赤のパッド、2列なのがLED青になります  
赤、青、赤、青・・・と交互に半田付けするのがやり易いです  
<img src="../image/image_buildguide_4.jpg" width=70% title="LED赤と青のパッド">  
LEDは方向があります。アノード→カソードの向きに電流が流れます  
逆に実装すると電流が流れず、LEDが点灯しません  
LEDはアノードが円の外側、カソードが円の内側になるように実装します  
LEDの左下部にカソードのマークがついています。このマークが円の内側になるように実装すれば、正しい向きに実装されます  
<img src="../image/image_buildguide_5.jpg" width=70% title="LED向き">  
<img src="../image/image_buildguide_5_2.JPG" width=70% title="LED向き">
  
## LEDドライバの半田付け  
次に基板裏側にLEDドライバを半田付けします    
<img src="../image/image_buildguide_6.jpg" width=70% title="LEDドライバ">  
基板裏面のU1～U9に実装します  
<img src="../image/image_buildguide_7.jpg" width=70% title="基板裏面">  
LEDドライバのマークと基板シルクのマークを合わせて実装します  
<img src="../image/image_buildguide_8.jpg" width=70% title="LEDドライバマーク">  
U1に乗せると以下のような向きになります  
<img src="../image/image_buildguide_9.jpg" width=70% title="U1にLEDドライバ">  
U1からU9まで8個実装します(U5はESP-WROOM-32)  
ドライバの足の間隔が1mmなので隣の足と半田ブリッジさせやすいです    
<img src="../image/image_buildguide_10.jpg" width=70% title="LEDドライバ実装">  
  
## ESP-WROOM-32の半田付け  
ESP-WROOM-32を半田付けします。ESP32と呼ばれてるWiFiモジュールです  
ファームウェアは既に書き込み済みです  
<img src="../image/image_buildguide_11.jpg" width=70% title="ESP-WROOM-32">   
半田付けが3辺あるので位置合わせは慎重に  
<img src="../image/image_buildguide_12.JPG" width=70% title="ESP-WROOM-32位置"> 
  
## 抵抗・コンデンサの半田付け  
抵抗、コンデンサを半田付けします  
10uのコンデンサは電源基板に2つ半田付け済みなので、1つしか入っていません  
<img src="../image/image_buildguide_13.JPG" width=70% title="抵抗,コンデンサ">  
0.1uのコンデンサをC1,C2,C4,C5,C6,c7,C8,C9,C10,C11の計10個実装します  
<img src="../image/image_buildguide_14.jpg" width=70% title="コンデンサ0.1u">   
10uのコンデンサをC3に実装します   
<img src="../image/image_buildguide_15.jpg" width=70% title="コンデンサ10u">  
10kの抵抗をR1～R11に11個実装します    
<img src="../image/image_buildguide_16.jpg" width=70% title="抵抗10k">  
  
## ピンヘッダの半田付け、電源基板、Tバーの組み立て  
電源基板を接続するためのピンヘッダを半田付けします  
以下のようなフィルムの中に入っています  
<img src="../image/image_buildguide_17.jpg" width=70% title="ピンヘッダ">  
  
J1のパッドに半田付けします  
<img src="../image/image_buildguide_18.jpg" width=70% title="J1">  
半田付けは以上です.  
J3のパッドですが、ESP32のファームウェア書き込み用です  
aitendoなどで1.27mmピッチの2列の[ピンヘッダ](http://www.aitendo.com/product/6880)、[ソケット](http://www.aitendo.com/product/11345)を購入して半田付けするとJ3のピンヘッダからファームウェアの変更ができるようになります  
  
スペーサー(ただのナノブロック)をLED基板裏に両面テープで貼り付けます  
スペーサーに両面テープは貼り付け済みです    
スペーサーを取り付けないと電源基板を接続したときに少しグラグラします  
<img src="../image/image_buildguide_19.jpg" width=70% title="スペーサー">  
スペーサー上部の両面テープの剥離紙を剥がして、J3に実装済みの電源基板を接続します  
<img src="../image/image_buildguide_20.jpg" width=70% title="電源基板接続">  
  
最後に基板とTバーの間にワッシャーをはさんで基板ごとネジで止めて完成です  
<img src="../image/image_buildguide_21.jpg" width=70% title="ワッシャー、Tバー">  
<img src="../image/image_buildguide_22.jpg" width=70% title="横から">  
<img src="../image/image_buildguide_23.jpg" width=70% title="裏から">  
  
D121のシルクがある位置でが０分の列になります   
この位置を真上にすると時間が読みやすくなります  
<img src="../image/image_buildguide_24.jpg" width=70% title="裏から">  
  
# 注意点  
電源基板のレギュレータはかなり熱くなります。手を触れないようにお願いします  
  






