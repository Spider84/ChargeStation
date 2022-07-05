/*
*********************************************************************************************************
*
*                   (c) Copyright 2003-2020, INITEC, Ekaterinburg. Russia.
*                                    All Rights Reserved
*
*
* Project     : none
*
* Filemane    : none
*
* Target      : Independent
* For         : ANSI C
*
* Description : none
*
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                         REVISION HISTORY
*
*
* Created     : Крылосов Е.Н.
* By          : 06.12.2020
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              INCLUDES
*********************************************************************************************************
*/

#include <PWM.h>
#include <Adafruit_NeoPixel.h>
#include "keyb.h"

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define OUT_NORMAL_PIN      29    // Выход "Норма".
#define OUT_CONNECT_PIN     30    // Выход "Подключен".
#define OUT_CHARGE_PIN      31    // Выход "Заряд".
#define OUT_CHARGE_FAN_PIN  32    // Выход "Заряд с вент".
#define OUT_ALARM_PIN       33    // Выход "Авария".

#define LED_NORMAL_PIN      2     // Светодиод "Норма".
#define LED_CONNECT_PIN     3     // Светодиод "Подключен".
#define LED_CHARGE_PIN      4     // Светодиод "Заряд".
#define LED_CHARGE_FAN_PIN  5     // Светодиод "Заряд с вент".
#define LED_ALARM_PIN       6     // Светодиод "Авария".

#define RELE_1_PIN          38    // Реле 1.
#define RELE_2_PIN          39    // Реле 2.
#define RELE_3_PIN          40    // Реле 3.

#define LED_RELE_1_PIN      7     // Светодиод "Реле 1".
#define LED_RELE_2_PIN      8     // Светодиод "Реле 2".
#define LED_RELE_3_PIN      9     // Светодиод "Реле 3".

#define IN_1_PIN            34    // Вход 1.
#define IN_2_PIN            35    // Вход 2.
#define IN_3_PIN            36    // Вход 3.
#define IN_4_PIN            37    // Вход 4.

#define LED_IN_1_PIN        13    // Светодиод "Вход 1".
#define LED_IN_2_PIN        22    // Светодиод "Вход 2".
#define LED_IN_3_PIN        23    // Светодиод "Вход 3".
#define LED_IN_4_PIN        24    // Светодиод "Вход 4".

#define LEDS_RGB_PIN        12    // Светодиод RGB.

#define PWM_OUT_PIN         11    // Выход ШИМ.
#define MSR_PWR_PIN         A0    // Вход измерения ШИМ.
#define MSR_R_PIN           A1    // Вход измерения сопротивления.


#define NUMPIXELS           90    // Количество пикселей RGB.

#define LED_LIGHT_MIN       0     // Минимальная яркость светодиодов.
#define LED_LIGHT_MAX       100   // Максимальная яркость светодиодов.

#define PWM_PERCENT         50    // Процент ШИМ.

/*
*********************************************************************************************************
*                                               MACROS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           LOCAL DATA TYPES
*********************************************************************************************************
*/

// Состояние электромобиля.
enum STATE_EVSE_E {
  STATE_ALARM = 0,
  STATE_NORMAL,
  STATE_CONNECT,
  STATE_CHARGE,
  STATE_CHARGE_FAN
};

// Режим пользователя.
enum USER_MODE_E {
  USER_MODE_NORMAL = 0,   // Норма.
  USER_MODE_PRE_CHARGE,   // Перед зарядом.
  USER_MODE_CHARGE,       // Заряд.
  USER_MODE_ALARM         // Авария.
};

/*
*********************************************************************************************************
*                                           LOCAL CLASSES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           LOCAL VARIABLES
*********************************************************************************************************
*/

Adafruit_NeoPixel pixels(NUMPIXELS, LEDS_RGB_PIN, NEO_GRB + NEO_KHZ800);

STATE_EVSE_E  stateNow;
STATE_EVSE_E  stateOld;
STATE_EVSE_E  stateHold;
int           countStateHold;

USER_MODE_E   userMode;       // Режим пользователя.

Keyb          keyb(IN_1_PIN, IN_2_PIN, IN_3_PIN, IN_4_PIN);

int           ledLight;           // Яркость светодиодов.
int           ledLightDelta = 1;  // Прирощение яркости светодиодов.

int           pwmCur;             // Текущий шим.

/*
*********************************************************************************************************
*                                           GLOBAL VARIABLES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             LOCAL TABLES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           GLOBAL FUNCTIONS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL FUNCTIONS
*********************************************************************************************************
*/

void setup() {
  Serial.begin(9600);

  pinMode(OUT_NORMAL_PIN,           OUTPUT);
  pinMode(OUT_CONNECT_PIN,          OUTPUT);
  pinMode(OUT_CHARGE_PIN,           OUTPUT);
  pinMode(OUT_CHARGE_FAN_PIN,       OUTPUT);
  pinMode(OUT_ALARM_PIN,            OUTPUT);
  
  pinMode(PWM_OUT_PIN,              OUTPUT);
  pinMode(RELE_1_PIN,               OUTPUT);
  pinMode(RELE_2_PIN,               OUTPUT);
  pinMode(RELE_3_PIN,               OUTPUT);

  pinMode(LED_NORMAL_PIN,           OUTPUT);
  pinMode(LED_CONNECT_PIN,          OUTPUT);
  pinMode(LED_CHARGE_PIN,           OUTPUT);
  pinMode(LED_CHARGE_FAN_PIN,       OUTPUT);
  pinMode(LED_ALARM_PIN,            OUTPUT);
  pinMode(LED_RELE_1_PIN,           OUTPUT);
  pinMode(LED_RELE_2_PIN,           OUTPUT);
  pinMode(LED_RELE_3_PIN,           OUTPUT);

  pinMode(IN_1_PIN,                 INPUT);
  pinMode(IN_2_PIN,                 INPUT);
  pinMode(IN_3_PIN,                 INPUT);
  pinMode(IN_4_PIN,                 INPUT);

  pinMode(LED_IN_1_PIN,             OUTPUT);
  pinMode(LED_IN_2_PIN,             OUTPUT);
  pinMode(LED_IN_3_PIN,             OUTPUT);
  pinMode(LED_IN_4_PIN,             OUTPUT);


  digitalWrite(RELE_1_PIN,          LOW);
  digitalWrite(RELE_2_PIN,          LOW);
  digitalWrite(RELE_3_PIN,          LOW);
  digitalWrite(LED_NORMAL_PIN,      HIGH);
  digitalWrite(LED_CONNECT_PIN,     HIGH);
  digitalWrite(LED_CHARGE_PIN,      HIGH);
  digitalWrite(LED_CHARGE_FAN_PIN,  HIGH);
  digitalWrite(LED_ALARM_PIN,       HIGH);
  digitalWrite(LED_RELE_1_PIN,      HIGH);
  digitalWrite(LED_RELE_2_PIN,      HIGH);
  digitalWrite(LED_RELE_3_PIN,      HIGH);
  digitalWrite(LED_IN_1_PIN,        HIGH);
  digitalWrite(LED_IN_2_PIN,        HIGH);
  digitalWrite(LED_IN_3_PIN,        HIGH);
  digitalWrite(LED_IN_4_PIN,        HIGH);

  digitalWrite(OUT_NORMAL_PIN,      HIGH);
  digitalWrite(OUT_CONNECT_PIN,     HIGH);
  digitalWrite(OUT_CHARGE_PIN,      HIGH);
  digitalWrite(OUT_CHARGE_FAN_PIN,  HIGH);
  digitalWrite(OUT_ALARM_PIN,       HIGH);

  //Timer1.initialize(100);
  //Timer1.attachInterrupt(timer1Isr);

  InitTimersSafe(); 
  bool success = SetPinFrequencySafe(PWM_OUT_PIN, 1000);

  pixels.begin();
  pixels.clear();
  pixels.show();

  userMode = USER_MODE_NORMAL;
  resetLedLight();
}

void loop() {
  pwmWrite(PWM_OUT_PIN, map(pwmCur, 0, 100, 0, 255));

  SetInLed(GetStateIn());

  stateNow = getState();

  if (stateNow == stateOld) {
    countStateHold++;
    if (countStateHold >= 5) {
      countStateHold = 0;
      stateHold = stateNow;
    }
  }
  else {
    countStateHold = 0;
  }
  stateOld = stateNow;

  keyb.run();
  refreshUserMode();

  // Показать текущий режим.
  switch(userMode) {
    case USER_MODE_NORMAL:
      Serial.println("USER_MODE_NORMAL");
      break;    
    case USER_MODE_PRE_CHARGE:
      Serial.println("USER_MODE_PRE_CHARGE");
      break;    
    case USER_MODE_CHARGE:
      Serial.println("USER_MODE_CHARGE");
      break;    
    case USER_MODE_ALARM:
      Serial.println("USER_MODE_ALARM");
      break;    
  }

  switch (stateHold) {
    case STATE_ALARM:
      //SetStateOut (0b00010000);
      SetStateLed (0b00010000);
      //SetRele     (0b00000000);
      //SetReleLed  (0b00000000);
      //putLight    (255, 0, 0, 100);
      break;

    case STATE_NORMAL:
      //SetStateOut (0b00000001);
      SetStateLed (0b00000001);
      //SetRele     (0b00000000);
      //SetReleLed  (0b00000000);
      //putLight    (0, 255, 0, 100);
      break;
    
    case STATE_CONNECT:
      //SetStateOut (0b00000010);
      SetStateLed (0b00000010);
      //SetRele     (0b00000000);
      //SetReleLed  (0b00000000);
      //putLight    (255, 255, 0, 100);
      break;

    case STATE_CHARGE:
      //SetStateOut (0b00000100);
      SetStateLed (0b00000100);
      //SetRele     (0b00000111);
      //SetReleLed  (0b00000001);
      //putLight    (0, 0, 255, 100);
      break;

    case STATE_CHARGE_FAN:
      //SetStateOut (0b00001000);
      SetStateLed (0b00001000);
      //SetRele     (0b00000111);
      //SetReleLed  (0b00000001);
      //putLight    (255, 255, 255, 100);
      break;
  }

  refreshLedLight();
}

void resetLedLight() {
  ledLight = LED_LIGHT_MAX;
  ledLightDelta = -1;
}

// Обновление яркости светодиодов.
void refreshLedLight() {
  ledLight += ledLightDelta;
  if ((ledLight >= LED_LIGHT_MAX) || (ledLight <= LED_LIGHT_MIN)) {
    ledLightDelta = -ledLightDelta;
  }
}

// Обновить пользовательский режим.
void refreshUserMode() {
  int event = keyb.getEvent();
  
  switch (userMode) {
    case USER_MODE_NORMAL:
      // Норма.
      if (event == KEYB_EVENT_PRESS_BTN_1) {
        // Нажали кнопку 1.          
        userMode = USER_MODE_PRE_CHARGE;
      }

      pwmCur = 0;

      SetStateOut (0b00000001);
      SetRele     (0b00000000);
      SetReleLed  (0b00000000);
      putLight    (0, 255, 0, ledLight);
      break;

    case USER_MODE_PRE_CHARGE:
      // Перед зарядом.
      if (event == KEYB_EVENT_PRESS_BTN_1) {
        // Нажали кнопку 1.          
        userMode = USER_MODE_NORMAL;
      }
      else {
        switch (stateHold) {
          case STATE_ALARM:
            userMode = USER_MODE_ALARM;
            break;
      
          case STATE_NORMAL:
            break;
          
          case STATE_CONNECT:
            break;
      
          case STATE_CHARGE:
            userMode = USER_MODE_CHARGE;
            break;
      
          case STATE_CHARGE_FAN:
            userMode = USER_MODE_CHARGE;
            break;
        }
      }

      pwmCur = PWM_PERCENT;

      SetStateOut (0b00000010);
      SetRele     (0b00000000);
      SetReleLed  (0b00000000);
      putLight    (255, 255, 0, LED_LIGHT_MAX);
      break;

    case USER_MODE_CHARGE:
      // Заряд.
      if (event == KEYB_EVENT_PRESS_BTN_1) {
        // Нажали кнопку 1.          
        userMode = USER_MODE_NORMAL;
      }
      else {
        switch (stateHold) {
          case STATE_ALARM:
            userMode = USER_MODE_ALARM;
            break;
      
          case STATE_NORMAL:
            userMode = USER_MODE_NORMAL;
            break;
          
          case STATE_CONNECT:
            userMode = USER_MODE_NORMAL;
            break;
      
          case STATE_CHARGE:
            break;
      
          case STATE_CHARGE_FAN:
            break;
        }
      }

      pwmCur = PWM_PERCENT;

      SetStateOut (0b00000100);
      SetRele     (0b00000001);
      SetReleLed  (0b00000001);
      putLight    (0, 0, 255, ledLight);
      
      break;
  
    case USER_MODE_ALARM:
      // Авария.
      if (event == KEYB_EVENT_PRESS_BTN_1) {
        // Нажали кнопку 1.          
        userMode = USER_MODE_NORMAL;
      }
      else {
        switch (stateHold) {
          case STATE_ALARM:
            break;
      
          case STATE_NORMAL:
            break;
          
          case STATE_CONNECT:
            break;
      
          case STATE_CHARGE:
            break;
      
          case STATE_CHARGE_FAN:
            break;
        }
      }

      pwmCur = PWM_PERCENT;

      SetStateOut (0b00001000);
      SetRele     (0b00000000);
      SetReleLed  (0b00000000);
      putLight    (255, 0, 0, LED_LIGHT_MAX);
      break;
  }

  keyb.resetEventFlag();
}

/*
void timer1Isr() {
  //PwmCount++;
  //if (PwmCount >= 10) PwmCount = 0;
  //GenOut = (PwmCount < DUTY_GEN) ? LOW : HIGH;
  //digitalWrite(GEN_PIN, GenOut);
  blink_cnt++;
}
*/

// Установка выходов состояния.
void SetStateOut(int state) {
  digitalWrite(OUT_NORMAL_PIN,      (state & 0x01) ? LOW : HIGH);
  digitalWrite(OUT_CONNECT_PIN,     (state & 0x02) ? LOW : HIGH);
  digitalWrite(OUT_CHARGE_PIN,      (state & 0x04) ? LOW : HIGH);
  digitalWrite(OUT_CHARGE_FAN_PIN,  (state & 0x08) ? LOW : HIGH);
  digitalWrite(OUT_ALARM_PIN,       (state & 0x10) ? LOW : HIGH);
}

// Установка светодиодов состояния.
void SetStateLed(int state) {
  digitalWrite(LED_NORMAL_PIN,      (state & 0x01) ? LOW : HIGH);
  digitalWrite(LED_CONNECT_PIN,     (state & 0x02) ? LOW : HIGH);
  digitalWrite(LED_CHARGE_PIN,      (state & 0x04) ? LOW : HIGH);
  digitalWrite(LED_CHARGE_FAN_PIN,  (state & 0x08) ? LOW : HIGH);
  digitalWrite(LED_ALARM_PIN,       (state & 0x10) ? LOW : HIGH);
}

// Установка реле.
void SetRele(int state) {
  digitalWrite(RELE_1_PIN,          (state & 0x01) ? HIGH : LOW);
  digitalWrite(RELE_2_PIN,          (state & 0x02) ? HIGH : LOW);
  digitalWrite(RELE_3_PIN,          (state & 0x04) ? HIGH : LOW);
}

// Установка светодиодов реле.
void SetReleLed(int state) {
  digitalWrite(LED_RELE_1_PIN,      (state & 0x01) ? LOW : HIGH);
  digitalWrite(LED_RELE_2_PIN,      (state & 0x02) ? LOW : HIGH);
  digitalWrite(LED_RELE_3_PIN,      (state & 0x04) ? LOW : HIGH);
}

// Установка светодиодов входов.
void SetInLed(int state) {
  digitalWrite(LED_IN_1_PIN,        (state & 0x01) ? LOW : HIGH);
  digitalWrite(LED_IN_2_PIN,        (state & 0x02) ? LOW : HIGH);
  digitalWrite(LED_IN_3_PIN,        (state & 0x04) ? LOW : HIGH);
  digitalWrite(LED_IN_4_PIN,        (state & 0x08) ? LOW : HIGH);
}

// Определение состояния входов.
int GetStateIn() {
  int i = 0;
  if (digitalRead(IN_1_PIN) == LOW) i |= 0x01;
  if (digitalRead(IN_2_PIN) == LOW) i |= 0x02;
  if (digitalRead(IN_3_PIN) == LOW) i |= 0x04;
  if (digitalRead(IN_4_PIN) == LOW) i |= 0x08;
  return i;
}

// Вывести свет.
void putLight(int r, int g, int b, int bright) {
  pixels.setBrightness(bright);
  pixels.fill(pixels.Color(r, g, b));
  pixels.show();
}

// Определение состояния электромобиля.
STATE_EVSE_E getState() {
  int min_v = 10000, max_v = -10000;
  int v;
  for (int i = 0; i < 100; i++) {
    v = analogRead(MSR_PWR_PIN);
    if (v < min_v) min_v = v;  
    if (v > max_v) max_v = v;  
  }

  //Serial.print("min=");
  //Serial.print(min_v);
  //Serial.print(", max=");
  //Serial.println(max_v);

  if (pwmCur == 0) return STATE_NORMAL; // ШИМ выключен.

  if (checkStateA(min_v, max_v)) return STATE_NORMAL;
  if (checkStateB(min_v, max_v)) return STATE_CONNECT;
  if (checkStateC(min_v, max_v)) return STATE_CHARGE;
  if (checkStateD(min_v, max_v)) return STATE_CHARGE_FAN;

  return STATE_ALARM;
}

// Проверка уровня.
boolean checkLevel(int level, int porog, int delta) {
  return ((level >= (porog - delta)) && (level <= (porog + delta)));
}

// Проверка уровней минимума и максимума.
boolean checkMinMax(int min_v, int porog_min, int max_v, int porog_max) {
  return ( checkLevel(min_v, porog_min, 50) && checkLevel(max_v, porog_max, 50) );
}

boolean checkStateA(int min_v, int max_v) {
  return (checkMinMax (min_v, 288, max_v, 1023));
}

boolean checkStateB(int min_v, int max_v) {
  return (checkMinMax (min_v, 288, max_v, 963));
}

boolean checkStateC(int min_v, int max_v) {
  return (checkMinMax (min_v, 288, max_v, 859));
}

boolean checkStateD(int min_v, int max_v) {
  return (checkMinMax (min_v, 288, max_v, 764));
}
