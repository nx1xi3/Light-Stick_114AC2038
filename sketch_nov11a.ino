// ===== 腳位定義 =====
const int BUTTON_PIN = 2;   // 按鈕腳位
const int RED_PIN = 9;      // RGB LED 紅色腳位 (需支援 PWM)
const int GREEN_PIN = 10;   // RGB LED 綠色腳位 (需支援 PWM)
const int BLUE_PIN = 11;    // RGB LED 藍色腳位 (需支援 PWM)

// ===== 變數設定 =====
int currentMode = 0;        // 0: 常亮模式 (Constant Light), 1: 閃爍模式 (Blink), 2: 呼吸燈模式 (Breathing)
int currentColorIndex = 0;  // 顏色索引 (0: 紅, 1: 黃, 2: 青)

// 當前燈光的顏色數值 (0 ~ 255)
int RCurrentColor = 255;
int GCurrentColor = 0;
int BCurrentColor = 0;

// 呼吸燈專用變數（改用獨立的亮度百分比變數來控制呼吸，避免直接亂加減造成卡死）
int breathValue = 0;        // 呼吸燈當前亮度 (0 ~ 255)
int fadeDirection = 5;      // 每次變化的量與方向 (正數變亮，負數變暗)

// 閃爍燈專用變數
unsigned long previousMillis = 0;
const long interval = 500;  // 閃爍間隔 (毫秒)
bool ledState = false;

// 按鈕長短按偵測變數
unsigned long pressStartTime = 0;
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
const unsigned long LONG_PRESS_TIME = 600; // 超過 0.6 秒視為長按

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP); // 啟用內建提升電阻
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  updateLEDColor(RCurrentColor, GCurrentColor, BCurrentColor);
}

void loop() {
  checkButton(); // 檢查按鈕狀態

  // 根據不同模式執行對應邏輯
  if (currentMode == 0) {
    // 0: 常亮模式 (Constant Light)
    updateLEDColor(RCurrentColor, GCurrentColor, BCurrentColor);
  } 
  else if (currentMode == 1) {
    // 1: 閃爍模式 (Blink)
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      ledState = !ledState;
      if (ledState) {
        updateLEDColor(RCurrentColor, GCurrentColor, BCurrentColor);
      } else {
        updateLEDColor(0, 0, 0); // 熄滅
      }
    }
  } 
  else if (currentMode == 2) {
    // 2: 呼吸燈模式 (Breathing Light)
    // 利用 breathValue (0 到 255) 乘上當前設定的顏色比例，達成完美的呼吸循環
    breathValue += fadeDirection;

    // 當亮度達到頂端 (255) 或底端 (0) 時，反轉方向
    if (breathValue >= 255) {
      breathValue = 255;
      fadeDirection = -fadeDirection; // 改為變暗
    } 
    else if (breathValue <= 0) {
      breathValue = 0;
      fadeDirection = -fadeDirection; // 改為變亮
    }

    // 根據當前選擇的顏色 (紅、黃、青)，依比例算出呼吸時的 RGB 數值
    int rOut = (RCurrentColor * breathValue) / 255;
    int gOut = (GCurrentColor * breathValue) / 255;
    int bOut = (BCurrentColor * breathValue) / 255;

    updateLEDColor(rOut, gOut, bOut);
    delay(20); // 控制呼吸速度
  }
}

// ===== 按鈕偵測函式 (長按切換模式，短按切換顏色) =====
void checkButton() {
  currentButtonState = digitalRead(BUTTON_PIN);

  // 按鈕按下 (低態有效)
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    pressStartTime = millis();
    delay(50); // 簡易消彈跳
  }

  // 按鈕釋放
  if (lastButtonState == LOW && currentButtonState == HIGH) {
    unsigned long pressDuration = millis() - pressStartTime;
    
    if (pressDuration >= LONG_PRESS_TIME) {
      // 長按：切換模式 (0 -> 1 -> 2 -> 0)
      currentMode = (currentMode + 1) % 3;
      breathValue = 0;     // 重設呼吸起點
      fadeDirection = 5;   // 重設為變亮方向
    } else {
      // 短按：切換顏色
      changeLEDColor();
    }
  }
  lastButtonState = currentButtonState;
}

// ===== 切換預設顏色的函式 (紅、黃、青) =====
void changeLEDColor() {
  currentColorIndex = (currentColorIndex + 1) % 3; // 循環 3 種顏色
  
  switch (currentColorIndex) {
    case 0: RCurrentColor = 255; GCurrentColor = 0;   BCurrentColor = 0;   break; // 紅色 (Red)
    case 1: RCurrentColor = 255; GCurrentColor = 255; BCurrentColor = 0;   break; // 黃色 (Yellow)
    case 2: RCurrentColor = 0;   GCurrentColor = 255; BCurrentColor = 255; break; // 青色 (Cyan)
  }
}

// ===== 更新實體 LED 輸出的函式 (含反相處理) =====
void updateLEDColor(int r, int g, int b) {
  // 如果你的 LED 是共陽極（Common Anode，即數值越低越亮），請把底下三行最前面的 `//` 刪除：
   r = 255 - r;
   g = 255 - g;
   b = 255 - b;

  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}