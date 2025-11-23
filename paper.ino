#include <M5GFX.h>
#include <Preferences.h>

M5GFX display;
Preferences prefs;

// 状態変数
int taskPoints = 10;      // 上段
int rewardSeconds = 0;    // 下段（秒単位で保存）
int slotResult = 0;       // 中段結果

// タイマー関連
bool timerRunning = false;
unsigned long timerStartMillis = 0;

// スロット倍率
int multipliers[4] = {0, 1, 2, 10};

// 画面分割（縦向き：540x960）
const int TOP_H = 320;
const int MID_H = 320;
const int BOT_H = 320;

// ボタン領域
struct Btn {
  int x, y, w, h;
};
Btn btnTaskAdd     = {170, 210, 200, 100};
Btn btnSpin        = {170, TOP_H + 210, 200, 100};
Btn btnTimer       = {170, TOP_H + MID_H + 210, 200, 100};


//-------------------------
// データの保存
//-------------------------
void saveData() {
  prefs.putInt("taskPts", taskPoints);
  prefs.putInt("rewardSec", rewardSeconds);
}

//-------------------------
// データの読み込み
//-------------------------
void loadData() {
  taskPoints = prefs.getInt("taskPts", 10);        // デフォルト10
  rewardSeconds = prefs.getInt("rewardSec", 0);    // デフォルト0
}


//-------------------------
// 現在の残り時間を取得
//-------------------------
int getCurrentSeconds() {
  if (timerRunning) {
    unsigned long elapsed = (millis() - timerStartMillis) / 1000;
    int remaining = rewardSeconds - elapsed;
    return max(0, remaining);
  } else {
    return rewardSeconds;
  }
}


//-------------------------
// 時間を「分:秒」形式で文字列化
//-------------------------
String formatTime(int totalSeconds) {
  int mins = totalSeconds / 60;
  int secs = totalSeconds % 60;
  return String(mins) + ":" + (secs < 10 ? "0" : "") + String(secs);
}


//-------------------------
// UI 描画
//-------------------------
void drawUI() {
  display.fillScreen(TFT_WHITE);
  display.setTextColor(TFT_BLACK);
  
  // 上段：タスクポイント ---------------------------------------------
  display.setTextSize(3);
  display.drawString("Task Points:", 30, 40);
  display.setTextSize(5);
  display.drawString(String(taskPoints), 30, 100);

  display.fillRect(btnTaskAdd.x, btnTaskAdd.y, btnTaskAdd.w, btnTaskAdd.h, TFT_DARKGREY);
  display.setTextColor(TFT_WHITE);
  display.setTextSize(6);
  display.drawString("+", btnTaskAdd.x + 80, btnTaskAdd.y + 30);

  // 中段：スロット ---------------------------------------------------
  display.setTextColor(TFT_BLACK);
  display.setTextSize(3);
  display.drawString("SLOT RESULT:", 30, TOP_H + 40);
  display.setTextSize(7);
  display.drawString("x" + String(slotResult), 220, TOP_H + 130);

  display.fillRect(btnSpin.x, btnSpin.y, btnSpin.w, btnSpin.h, TFT_BLACK);
  display.setTextColor(TFT_WHITE);
  display.setTextSize(4);
  display.drawString("SPIN", btnSpin.x + 45, btnSpin.y + 35);

  // 下段：ご褒美時間 -------------------------------------------------
  display.setTextColor(TFT_BLACK);
  display.setTextSize(3);
  display.drawString("Reward Time:", 30, TOP_H + MID_H + 40);
  
  // 常に「分:秒」形式で表示
  int currentSec = getCurrentSeconds();
  display.setTextSize(5);
  display.drawString(formatTime(currentSec), 30, TOP_H + MID_H + 100);

  // スタート/ストップボタン
  if (timerRunning) {
    display.fillRect(btnTimer.x, btnTimer.y, btnTimer.w, btnTimer.h, TFT_RED);
    display.setTextColor(TFT_WHITE);
    display.setTextSize(4);
    display.drawString("STOP", btnTimer.x + 45, btnTimer.y + 35);
  } else {
    display.fillRect(btnTimer.x, btnTimer.y, btnTimer.w, btnTimer.h, TFT_GREEN);
    display.setTextColor(TFT_WHITE);
    display.setTextSize(4);
    display.drawString("START", btnTimer.x + 35, btnTimer.y + 35);
  }

  display.display();  // E-Ink に反映
}


//-------------------------
// スロット演出（リールが回る）
//-------------------------
void slotAnimation(int finalResult) {
  // リール高速回転（期待感）
  for (int i = 0; i < 15; i++) {
    // 背景クリア（スロット部分のみ）
    display.fillRect(0, TOP_H, 540, 320, TFT_WHITE);
    
    // ランダムな数値を表示
    int randNum = multipliers[random(4)];
    display.setTextColor(TFT_BLACK);
    display.setTextSize(7);
    display.drawString("x" + String(randNum), 220, TOP_H + 130);
    
    display.display();
    delay(80 - i * 3);  // だんだん遅くなる
  }
  
  // 少し間を置く（緊張感）
  delay(200);
  
  // 最終結果の演出
  display.fillRect(0, TOP_H, 540, 320, TFT_WHITE);
  
  // 結果の色を変える（大当たりは派手に）
  if (finalResult == 10) {
    // 10倍は特別演出
    for (int flash = 0; flash < 3; flash++) {
      display.fillRect(0, TOP_H, 540, 320, TFT_BLACK);
      display.setTextColor(TFT_WHITE);
      display.setTextSize(8);
      display.drawString("x10!!", 180, TOP_H + 130);
      display.display();
      delay(150);
      
      display.fillRect(0, TOP_H, 540, 320, TFT_WHITE);
      display.setTextColor(TFT_BLACK);
      display.setTextSize(8);
      display.drawString("x10!!", 180, TOP_H + 130);
      display.display();
      delay(150);
    }
  } else if (finalResult == 0) {
    // ハズレ演出
    display.setTextColor(TFT_DARKGREY);
    display.setTextSize(7);
    display.drawString("x0", 220, TOP_H + 130);
    display.setTextSize(3);
    display.drawString("miss...", 200, TOP_H + 180);
    display.display();
    delay(800);
  } else {
    // 通常の当たり
    display.setTextColor(TFT_BLACK);
    display.setTextSize(7);
    display.drawString("x" + String(finalResult), 220, TOP_H + 130);
    display.display();
    delay(600);
  }
}


//-------------------------
// タイマー更新
//-------------------------
void updateTimer() {
  if (!timerRunning) return;
  
  int currentSec = getCurrentSeconds();
  
  if (currentSec <= 0) {
    // タイマー終了
    timerRunning = false;
    rewardSeconds = 0;
    saveData();
    drawUI();
  }
}


//---------------------------------------------
// セットアップ
//---------------------------------------------
void setup() {
  display.init();
  display.startWrite();

  if (display.isEPD()) {
    display.setEpdMode(epd_mode_t::epd_fastest);
  }
  
  // 縦向き（540x960）に設定
  display.setRotation(0);
  
  // Preferences初期化（データ永続化用）
  prefs.begin("slotgame", false);
  
  // 保存されたデータを読み込み
  loadData();
  
  randomSeed(esp_timer_get_time());
  
  drawUI();
}


//---------------------------------------------
// メインループ：タッチ処理
//---------------------------------------------
void loop() {
  // タイマー更新（1秒ごと）
  static unsigned long lastUpdate = 0;
  if (timerRunning && millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    updateTimer();
    drawUI();
  }
  
  lgfx::v1::touch_point_t t;
  uint_fast8_t cnt = display.getTouch(&t, 1);

  if (cnt > 0) {
    int tx = t.x;
    int ty = t.y;

    // タスク追加ボタン
    if (tx > btnTaskAdd.x && tx < btnTaskAdd.x + btnTaskAdd.w &&
        ty > btnTaskAdd.y && ty < btnTaskAdd.y + btnTaskAdd.h) {
      taskPoints++;
      saveData();
      drawUI();
      delay(300);
    }

    // スロット回すボタン
    if (tx > btnSpin.x && tx < btnSpin.x + btnSpin.w &&
        ty > btnSpin.y && ty < btnSpin.y + btnSpin.h) {
      if (taskPoints > 0) {
        taskPoints--;
        
        // スロット結果を決定
        int r = random(100);
        if (r < 5) slotResult = 0;          // 5%
        else if (r < 55) slotResult = 1;    // 50%
        else if (r < 80) slotResult = 2;    // 25%
        else slotResult = 10;               // 20%
        
        // 演出実行！
        slotAnimation(slotResult);
        
        // 1ポイント = 1分（60秒）を加算
        int currentSec = getCurrentSeconds();
        int addedSeconds = slotResult * 60;
        
        if (timerRunning) {
          // タイマー動作中：現在の残り時間に追加
          rewardSeconds = currentSec + addedSeconds;
          timerStartMillis = millis();  // タイマーをリセット
        } else {
          // タイマー停止中：保存された時間に追加
          rewardSeconds = currentSec + addedSeconds;
        }
        
        saveData();
      }
      drawUI();
      delay(300);
    }

    // タイマー スタート/ストップボタン
    if (tx > btnTimer.x && tx < btnTimer.x + btnTimer.w &&
        ty > btnTimer.y && ty < btnTimer.y + btnTimer.h) {
      if (timerRunning) {
        // タイマー停止：現在の残り時間を保存
        rewardSeconds = getCurrentSeconds();
        timerRunning = false;
        saveData();
      } else if (rewardSeconds > 0) {
        // タイマー開始
        timerRunning = true;
        timerStartMillis = millis();
      }
      drawUI();
      delay(300);
    }
  }
}