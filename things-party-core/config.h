#define BTSerialRX 52
#define BTSerialTX 53

#define ServoPinA 9
#define ServoPinB 10

// HC-SR04のPIN 水平
#define trigPinA 48
#define echoPinA 49
// スタートからの距離のリミット(cm)
#define limitMinPinA 6
#define limitMaxPinA 13

// HC-SR04のPIN　垂直
#define trigPinB 50
#define echoPinB 51
// 地面からの距離のリミット(cm)
#define limitMinPinB 1
#define limitMaxPinB 10

// 0 = スタート
// 1 = クレーン右 *
// 2 = 水平移動ストップ *
// 3 = アーム回転 *
// 4 = アームストップ
// 5 = クレーン下
// 6 =
// 7 = アーム閉じる＆クレーン上 *
// 8 = クレーン上
// 9 = クレーン左
#define FlgStart 0
#define FlgRight 1
#define FlgStop 2
#define FlgRotate 3
#define FlgDown 5
#define FlgCatch 7
#define FlgUp 8
#define FlgLeft 9

//3色LED
#define LedRed 23
#define LedBlue 25
#define LedGreen 27
