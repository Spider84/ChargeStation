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
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <EEPROM.h>
#include <Watchdog.h>
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

#define KEY_NFC             IN_2_PIN
#define KEY_START           IN_1_PIN
#define BEEP_PIN            41

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
  USER_MODE_ALARM,        // Авария.
  USER_MODE_CARD_ADD,
  USER_MODE_CARD_CLEAR
};

typedef uint8_t tag_t[7];

typedef struct {
    uint8_t flag;
    tag_t tag;
} tag_recotrd;

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

STATE_EVSE_E stateNow;
STATE_EVSE_E stateOld;
STATE_EVSE_E stateHold;
int countStateHold;

USER_MODE_E userMode; // Режим пользователя.

Keyb keyb(IN_1_PIN, IN_2_PIN, IN_3_PIN, IN_4_PIN);

int ledLight; // Яркость светодиодов.
int ledLightDelta = 1; // Прирощение яркости светодиодов.

int pwmCur; // Текущий шим.

/*
*********************************************************************************************************
*                                           GLOBAL VARIABLES
*********************************************************************************************************
*/

static PN532_I2C pn532_i2c(Wire);
static NfcAdapter nfc = NfcAdapter(pn532_i2c);
static NfcTag tag;
bool granted = false;
Watchdog watchdog;

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

void setup()
{
    Serial.begin(230400);

    pinMode(OUT_NORMAL_PIN, OUTPUT);
    pinMode(OUT_CONNECT_PIN, OUTPUT);
    pinMode(OUT_CHARGE_PIN, OUTPUT);
    pinMode(OUT_CHARGE_FAN_PIN, OUTPUT);
    pinMode(OUT_ALARM_PIN, OUTPUT);

    pinMode(PWM_OUT_PIN, OUTPUT);
    pinMode(RELE_1_PIN, OUTPUT);
    pinMode(RELE_2_PIN, OUTPUT);
    pinMode(RELE_3_PIN, OUTPUT);

    pinMode(LED_NORMAL_PIN, OUTPUT);
    pinMode(LED_CONNECT_PIN, OUTPUT);
    pinMode(LED_CHARGE_PIN, OUTPUT);
    pinMode(LED_CHARGE_FAN_PIN, OUTPUT);
    pinMode(LED_ALARM_PIN, OUTPUT);
    pinMode(LED_RELE_1_PIN, OUTPUT);
    pinMode(LED_RELE_2_PIN, OUTPUT);
    pinMode(LED_RELE_3_PIN, OUTPUT);

    pinMode(IN_1_PIN, INPUT);
    pinMode(IN_2_PIN, INPUT);
    pinMode(IN_3_PIN, INPUT);
    pinMode(IN_4_PIN, INPUT);

    pinMode(LED_IN_1_PIN, OUTPUT);
    pinMode(LED_IN_2_PIN, OUTPUT);
    pinMode(LED_IN_3_PIN, OUTPUT);
    pinMode(LED_IN_4_PIN, OUTPUT);
    pinMode(BEEP_PIN, OUTPUT);

    digitalWrite(RELE_1_PIN, LOW);
    digitalWrite(RELE_2_PIN, LOW);
    digitalWrite(RELE_3_PIN, LOW);
    digitalWrite(LED_NORMAL_PIN, HIGH);
    digitalWrite(LED_CONNECT_PIN, HIGH);
    digitalWrite(LED_CHARGE_PIN, HIGH);
    digitalWrite(LED_CHARGE_FAN_PIN, HIGH);
    digitalWrite(LED_ALARM_PIN, HIGH);
    digitalWrite(LED_RELE_1_PIN, HIGH);
    digitalWrite(LED_RELE_2_PIN, HIGH);
    digitalWrite(LED_RELE_3_PIN, HIGH);
    digitalWrite(LED_IN_1_PIN, HIGH);
    digitalWrite(LED_IN_2_PIN, HIGH);
    digitalWrite(LED_IN_3_PIN, HIGH);
    digitalWrite(LED_IN_4_PIN, HIGH);

    digitalWrite(OUT_NORMAL_PIN, HIGH);
    digitalWrite(OUT_CONNECT_PIN, HIGH);
    digitalWrite(OUT_CHARGE_PIN, HIGH);
    digitalWrite(OUT_CHARGE_FAN_PIN, HIGH);
    digitalWrite(OUT_ALARM_PIN, HIGH);

    digitalWrite(BEEP_PIN, LOW);

    // Setup watchdog
    watchdog.enable(Watchdog::TIMEOUT_8S);
    watchdog.reset();

    // Timer1.initialize(100);
    // Timer1.attachInterrupt(timer1Isr);

    InitTimersSafe();

    nfc.begin();
    watchdog.reset();

    bool success = SetPinFrequencySafe(PWM_OUT_PIN, 1000);

    pixels.begin();
    pixels.clear();
    pixels.show();

    userMode = USER_MODE_NORMAL;
    resetLedLight();
    watchdog.reset();
}

void loop()
{
    watchdog.reset();
    readNFC();
    pwmWrite(PWM_OUT_PIN, map(pwmCur, 0, 100, 0, 255));

    SetInLed(GetStateIn());

    stateNow = getState();

    if (stateNow == stateOld) {
        countStateHold++;
        if (countStateHold >= 5) {
            countStateHold = 0;
            stateHold = stateNow;
        }
    } else {
        countStateHold = 0;
    }
    stateOld = stateNow;

    keyb.run();
    refreshUserMode();

    // Показать текущий режим.
    static uint32_t last_show = 0;
    uint32_t now = millis();
    if (now - last_show>1000) {
      last_show = now;
      switch (userMode) {
      case USER_MODE_NORMAL:
          Serial.println(F("USER_MODE_NORMAL"));
          break;
      case USER_MODE_PRE_CHARGE:
          Serial.println(F("USER_MODE_PRE_CHARGE"));
          break;
      case USER_MODE_CHARGE:
          Serial.println(F("USER_MODE_CHARGE"));
          break;
      case USER_MODE_ALARM:
          Serial.println(F("USER_MODE_ALARM"));
          break;
      case USER_MODE_CARD_ADD:
          Serial.println(F("USER_MODE_CARD_ADD"));
          break;
      case USER_MODE_CARD_CLEAR:
          Serial.println(F("USER_MODE_CARD_CLEAR"));
          clear_tags();
          userMode = USER_MODE_NORMAL;
          break;
      }
    }

    switch (stateHold) {
    case STATE_ALARM:
        // SetStateOut (0b00010000);
        SetStateLed(0b00010000);
        // SetRele     (0b00000000);
        // SetReleLed  (0b00000000);
        // putLight    (255, 0, 0, 100);
        break;

    case STATE_NORMAL:
        // SetStateOut (0b00000001);
        SetStateLed(0b00000001);
        // SetRele     (0b00000000);
        // SetReleLed  (0b00000000);
        // putLight    (0, 255, 0, 100);
        break;

    case STATE_CONNECT:
        // SetStateOut (0b00000010);
        SetStateLed(0b00000010);
        // SetRele     (0b00000000);
        // SetReleLed  (0b00000000);
        // putLight    (255, 255, 0, 100);
        break;

    case STATE_CHARGE:
        // SetStateOut (0b00000100);
        SetStateLed(0b00000100);
        // SetRele     (0b00000111);
        // SetReleLed  (0b00000001);
        // putLight    (0, 0, 255, 100);
        break;

    case STATE_CHARGE_FAN:
        // SetStateOut (0b00001000);
        SetStateLed(0b00001000);
        // SetRele     (0b00000111);
        // SetReleLed  (0b00000001);
        // putLight    (255, 255, 255, 100);
        break;
    }

    refreshLedLight();
}

void clear_tags()
{
    EEPROM.update(0, 0xFF);
}

void print_tag(tag_t tag)
{
    Serial.print((unsigned int)sizeof(tag_t));
    for (uint8_t i = 0; i < sizeof(tag_t); ++i) {
        Serial.print(':');
        uint8_t c = tag[i];
        if (c < 0x10)
            Serial.print('0');
        Serial.print(c, HEX);
    }
    Serial.println();
}

bool find_tag(tag_t tag)
{
    tag_recotrd tmp_tag;
    uint32_t ee_count = EEPROM.length();
    //  tag_t tag;
    //  memset(tag, 0xFF, sizeof(tag_t));
    //  nfc_tag.getUid((byte *)tag, min(sizeof(tag_t),nfc_tag.getUidLength()));
    // print_tag(tag);
    for (uint32_t addr = 0; addr < ee_count; addr += sizeof(tag_recotrd)) {
        // Serial.println(addr, HEX);
        EEPROM.get(addr, tmp_tag);
        // Serial.print(tmp_tag.flag, HEX);
        // Serial.print(" ");
        // print_tag(tmp_tag.tag);
        if ((tmp_tag.flag & 0x03) == 2) {
            if (!memcmp(tag, tmp_tag.tag, sizeof(tag_t))) {
                Serial.println(F("FOUND!"));
                return true;
            }
        } else if (tmp_tag.flag == 0xFFU) {
            break;
        }
    }
    Serial.println(F("NOT FOUND!"));
    return false;
}

bool add_tag(tag_t tag)
{
    tag_recotrd tmp_tag;
    uint32_t ee_count = EEPROM.length();
    //    tag_t tag;
    //    memset(tag, 0xFF, sizeof(tag_t));
    //    nfc_tag.getUid((byte *)tag, min(sizeof(tag_t),nfc_tag.getUidLength()));
    // print_tag(tag);
    for (uint32_t addr = 0; addr < ee_count; addr += sizeof(tag_recotrd)) {
        // Serial.println(addr, HEX);
        EEPROM.get(addr, tmp_tag);
        // Serial.print(tmp_tag.flag, HEX);
        // Serial.print(" ");
        // print_tag(tmp_tag.tag);
        bool is_last = tmp_tag.flag == 0xFF;
        if (is_last || ((tmp_tag.flag & 0x02) == 0)) {
            tmp_tag.flag = (uint8_t)0xFE;
            memcpy(tmp_tag.tag, tag, sizeof(tag_t));
            EEPROM.put(addr, tmp_tag);
            if (is_last) {
                // Serial.println("LAST ");
                EEPROM.update(addr + sizeof(tag_recotrd), 0xFF);
            }
            return true;
        }
    }
    return false;
}

//Весь ужас далее - попытка придать стандартному PN532 коду большей асинхронности
//Из коробки он блокируемый, что приводит к тормозам кода и реакциям на кнопку.

uint8_t pn532_packetbuffer[64];

void readNFC()
{
    static tag_t last_tag;
    static uint32_t tag_time = 0;
    static uint8_t s_state = 0;
    int8_t ret = -1;

    switch (s_state) {
    default:
        s_state = 0;
    case 0:
        //      Serial.print("CMD ");
        pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
        pn532_packetbuffer[1] = 1; // max 1 cards at once (we can set this to 2 later)
        pn532_packetbuffer[2] = PN532_MIFARE_ISO14443A;

        ret = pn532_i2c.writeCommand(pn532_packetbuffer, 3);
        if (ret >= 0)
            ++s_state;
        break;
    case 1:
        ret = pn532_i2c.readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1);
        // Serial.print("RESP: ");Serial.println(ret);
        if (ret != -1) {
            ++s_state;
            if (ret >= 0) {
                ret = -1;
                if (pn532_packetbuffer[0] == 1) {
                    //          uint16_t sens_res = pn532_packetbuffer[2] << 8 | pn532_packetbuffer[3];
                    //          DMSG("ATQA: 0x");  DMSG_HEX(sens_res);
                    //          DMSG("SAK: 0x");  DMSG_HEX(pn532_packetbuffer[4]);
                    //          DMSG("\n");

                    /* Card appears to be Mifare Classic */
                    //          *uidLength = pn532_packetbuffer[5];

                    tag_t tmp_tag;
                    memset(tmp_tag, 0xFF, sizeof(tag_t));
                    for (uint8_t i = 0; i < pn532_packetbuffer[5]; i++) {
                        tmp_tag[i] = pn532_packetbuffer[6 + i];
                    }

                    if (!memcmp(last_tag, tmp_tag, sizeof(tag_t)) && (millis() - tag_time < 5000)) {
                        return;
                    }
                    tag_time = millis();

                    //   Serial.println("OTHER");
                    memcpy(last_tag, tmp_tag, sizeof(tag_t));

                    Serial.print(F("Readed MAC: "));print_tag(tmp_tag);

                    switch (userMode) {
                    case USER_MODE_NORMAL:
                        // Serial.println("BEEP");
                        digitalWrite(BEEP_PIN, HIGH);
                        delay(200);
                        digitalWrite(BEEP_PIN, LOW);

                        Serial.print(F("Re-Read CARD..."));
                        if (nfc.tagPresent()) {
                            granted = false;

                            Serial.print(F("Found..."));
                            tag = nfc.read();
                            Serial.println(F("Readed"));

                            uint8_t len = min(sizeof(tag_t), tag.getUidLength());
                            memset(tmp_tag, 0xFF, sizeof(tag_t));
                            tag.getUid((byte*)tmp_tag, len);

                            Serial.print(F("Readed MAC: "));print_tag(tmp_tag);
                            for (unsigned int i = 0; i < len; ++i) {
                                tmp_tag[i] = ~tmp_tag[i];
                            }
                            Serial.print(F("Invert MAC: "));print_tag(tmp_tag);

                            //  Serial.print("Tag Type: ");Serial.println(tag.getTagType());
                            //  Serial.print("UID: ");Serial.println(tag.getUidString());

                            if (tag.hasNdefMessage()) {
                                NdefMessage ndef(tag.getNdefMessage());
                                unsigned int cnt(ndef.getRecordCount());
                                Serial.print(F("NDEFs count: "));
                                Serial.println(cnt);
                                for (unsigned int i = 0; i < cnt; ++i) {
                                    NdefRecord rec(ndef.getRecord(i));
                                    byte Tnf(rec.getTnf());
                                    Serial.print(i);
                                    Serial.print(F(" TNF: "));
                                    Serial.print(Tnf);
                                    if (Tnf == TNF_WELL_KNOWN) {
                                        unsigned int len(rec.getTypeLength());
                                        Serial.print(F(" Len: "));Serial.print(len);
                                        if (len == 1) {
                                            uint8_t RTD_TYPE = 0x00;
                                            rec.getType(&RTD_TYPE);
                                            Serial.print(F(" Type: "));Serial.print(RTD_TYPE);
                                            if (RTD_TYPE == 'B') {
                                                unsigned int len(rec.getPayloadLength());
                                                Serial.print(F(" PLLen: "));Serial.print(len);
                                                if (len <= sizeof(tag_t)) {
                                                    rec.getPayload(tmp_tag);
                                                    Serial.print(F(" NDEF Tag: "));print_tag(tmp_tag);

                                                    if (find_tag(tmp_tag)) {
                                                       granted = true;
                                                       break;
                                                    }
                                                } else
                                                    Serial.println();
                                            }
                                        }
                                    }
                                    Serial.println();
                                }
                            } else
                                Serial.println(F("No NDEF Records"));
                        }
                        if (granted) {
                            // Serial.println("BEEP2");
                            delay(200);
                            digitalWrite(BEEP_PIN, HIGH);
                            delay(200);
                            digitalWrite(BEEP_PIN, LOW);
                        }
                        break;
                    case USER_MODE_CARD_ADD:
                        userMode = USER_MODE_NORMAL;
                        digitalWrite(BEEP_PIN, HIGH);
                        delay(200);
                        digitalWrite(BEEP_PIN, LOW);
                        delay(200);
                        digitalWrite(BEEP_PIN, HIGH);

                        Serial.println(F("Try to Write NDEF..."));
                        Serial.print(F("Re-Read CARD..."));
                        if (nfc.tagPresent()) {
                            Serial.print(F("Found..."));
                            tag = nfc.read();
                            Serial.println(F("Readed"));
                            //  Serial.println("NFC Read");

                            //  Serial.print("Tag Type: ");Serial.println(tag.getTagType());
                            //  Serial.print("UID: ");Serial.println(tag.getUidString());

                            uint8_t len = min(sizeof(tag_t), tag.getUidLength());
                            memset(tmp_tag, 0xFF, sizeof(tag_t));
                            tag.getUid((byte*)tmp_tag, len);

                            Serial.print(F("Readed MAC: "));print_tag(tmp_tag);

                            for (unsigned int i = 0; i < len; ++i) {
                                tmp_tag[i] = ~tmp_tag[i];
                            }

                            Serial.print(F("Invert MAC: "));print_tag(tmp_tag);
                            // Serial.print(F("Re-Read CARD..."));
                            // if (nfc.tagPresent()) {
                            //   Serial.print(F("Found..."));
                            //   tag = nfc.read();
                            //   Serial.println(F("Readed"));

                            NdefMessage ndef((!tag.hasNdefMessage()) ? NdefMessage() : tag.getNdefMessage());
                            if (!tag.hasNdefMessage() || (ndef.getRecordCount() >= MAX_NDEF_RECORDS)) {
                                Serial.println(F("Trying to clean..."));
                                
                                Serial.print(F("Re-Read CARD..."));
                                if (nfc.tagPresent()) {
                                  Serial.print(F("Found..."));
                                  bool cleared = false;
                                  uint8_t try_num = 0;
                                  while (!(cleared = nfc.clean())) {
                                      Serial.println(F("ERR Unable to clean!"));
                                      if (++try_num>5)
                                        break;
                                  }
                                  if (cleared) {
                                    Serial.println(F("Cleared!"));
                                  }
                                }
                                Serial.print(F("Re-Read CARD..."));
                                if (nfc.tagPresent()) {
                                    Serial.print(F("Found..."));
                                    Serial.println(F("Trying to format..."));
                                    bool cleared = false;
                                    uint8_t try_num = 0;
                                    while (!(cleared = nfc.format())) {
                                        Serial.println(F("ERR Unable to format!"));
                                        if (++try_num>5)
                                          break;
                                    }
                                    if (cleared) {
                                       Serial.println(F("Formated!"));
                                    }
                                }
                                // Serial.println(F("TAG Formated!"));
                                // uint8_t cnt = 10;
                                // while(cnt-->0 && !nfc.tagPresent(100));
                            } else {
                                Serial.println(F("Good NDEF"));
                            }

                             ndef.addRecord({(byte)TNF_WELL_KNOWN, (const byte*)"B", (size_t)1, (const byte*)tmp_tag, sizeof(tag_t)});
//                             NdefRecord *r(new NdefRecord());
//                             r->setTnf(TNF_WELL_KNOWN);
//                             r->setType((const byte *)"B", 1);
//                             r->setPayload(tmp_tag, sizeof(tag_t));
//                             ndef.addRecord(*r);
//                             delete r;

                            Serial.print(F("NFC Write "));
                            boolean writed(nfc.write(ndef));
                            if (writed) {
                                Serial.println(F("DONE!"));
                                goto done;
                            } else {
                                Serial.println(F("FAIL!"));
                            }
                            for (uint8_t i = 0; i < 2; ++i) {
                                digitalWrite(BEEP_PIN, LOW);
                                delay(500);
                                digitalWrite(BEEP_PIN, HIGH);
                                delay(500);
                            }
                        }
                    done:
                        digitalWrite(BEEP_PIN, LOW);
                        add_tag(tmp_tag);

                        delay(2000);
                        break;
                    }
                }
            }
        }
        break;
    }
    //  Serial.print("RET:");
    //  Serial.println(ret);
}

void resetLedLight()
{
    ledLight = LED_LIGHT_MAX;
    ledLightDelta = -1;
}

// Обновление яркости светодиодов.
void refreshLedLight()
{
    ledLight += ledLightDelta;
    if ((ledLight >= LED_LIGHT_MAX) || (ledLight <= LED_LIGHT_MIN)) {
        ledLightDelta = -ledLightDelta;
    }
}

// Обновить пользовательский режим.
void refreshUserMode()
{
    static int event = 0;
    static unsigned long pressTime;
    static unsigned char pressCnt = 0;

    if (keyb.getEventFlag()) {
        event = keyb.getEvent();
    }

    //  Serial.print(keyb.getEventFlag());
    //  Serial.print(" ");
    //  Serial.println(event);

    switch (userMode) {
    case USER_MODE_CARD_CLEAR:
        putLight(255, 0, 0, LED_LIGHT_MAX);
        break;
    case USER_MODE_CARD_ADD:
        if (event == KEYB_EVENT_PRESS_BTN_2) {
            if (pressTime && (millis() - pressTime >= 60000)) {
                userMode = USER_MODE_ALARM;
                pressTime = 0;
            }
        } else {
            if (pressTime && (millis() - pressTime >= 5000)) { // 5000
                userMode = USER_MODE_NORMAL;
                pressTime = 0;
            }
        }
        putLight(255, 0, 255, LED_LIGHT_MAX);
        break;
    case USER_MODE_NORMAL:
        // Норма.
        if (event == KEYB_EVENT_PRESS_BTN_1) {
            if (keyb.getEventFlag()) {
                // Нажали кнопку 1.
                if (granted)
                    userMode = USER_MODE_PRE_CHARGE;
                granted = false;
            }
        } else if (event == KEYB_EVENT_PRESS_BTN_2) {
            if (keyb.getEventFlag()) {
                // Нажали кнопку 2.
                pressTime = millis();
                //          Serial.println("BTN2_PRESS");
            } else {
                //          Serial.print("BTN2 ");
                //          Serial.println(millis() - pressTime);
                if (pressTime && (millis() - pressTime >= 5000)) {
                    Serial.println(F("BTN2_HOLD"));
                    userMode = USER_MODE_CARD_ADD;
                    pressTime = millis();
                    Serial.println(F("BEEP"));
                    for (uint8_t i = 0; i < 3; ++i) {
                        digitalWrite(BEEP_PIN, HIGH);
                        delay(200);
                        digitalWrite(BEEP_PIN, LOW);
                        delay(200);
                    }
                }
            }
        } else if (event == KEYB_EVENT_UN_PRESS_BTN_2) {
            if (keyb.getEventFlag()) {
                //          Serial.println("BTN2_UN_PRESS");
                if (pressTime && (millis() - pressTime < 1000)) {
                    ++pressCnt;
                    pressTime = millis();
                }
            }
        }

        if (pressCnt && (millis() - pressTime >= 1000)) {
            // Serial.print("CNT ");
            // Serial.println(pressCnt);
            if (pressCnt == 5) {
                putLight(255, 0, 0, LED_LIGHT_MAX);                
                userMode = USER_MODE_CARD_CLEAR;
                Serial.println(F("CLEAR"));
                for (uint8_t i = 0; i < 6; ++i) {
                    digitalWrite(BEEP_PIN, HIGH);
                    delay(200);
                    digitalWrite(BEEP_PIN, LOW);
                    delay(200);
                }
            }
            //        delay(2000);
            pressCnt = 0;
        }

        pwmCur = 0;

        SetStateOut(0b00000001);
        SetRele(0b00000000);
        SetReleLed(0b00000000);
        putLight(0, 255, 0, ledLight);
        break;

    case USER_MODE_PRE_CHARGE:
        // Перед зарядом.
        if (event == KEYB_EVENT_PRESS_BTN_1 && keyb.getEventFlag()) {
            // Нажали кнопку 1.
            userMode = USER_MODE_NORMAL;
            granted = false;
        } else {
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

        SetStateOut(0b00000010);
        SetRele(0b00000000);
        SetReleLed(0b00000000);
        putLight(255, 255, 0, LED_LIGHT_MAX);
        break;

    case USER_MODE_CHARGE:
        // Заряд.
        if (event == KEYB_EVENT_PRESS_BTN_1 && keyb.getEventFlag()) {
            // Нажали кнопку 1.
            userMode = USER_MODE_NORMAL;
        } else {
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

        SetStateOut(0b00000100);
        SetRele(0b00000001);
        SetReleLed(0b00000001);
        putLight(0, 0, 255, ledLight);

        break;

    case USER_MODE_ALARM:
        // Авария.
        if (event == KEYB_EVENT_PRESS_BTN_1 && keyb.getEventFlag()) {
            // Нажали кнопку 1.
            userMode = USER_MODE_NORMAL;
        } else {
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

        SetStateOut(0b00001000);
        SetRele(0b00000000);
        SetReleLed(0b00000000);
        putLight(255, 0, 0, LED_LIGHT_MAX);
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
void SetStateOut(int state)
{
    digitalWrite(OUT_NORMAL_PIN, (state & 0x01) ? LOW : HIGH);
    digitalWrite(OUT_CONNECT_PIN, (state & 0x02) ? LOW : HIGH);
    digitalWrite(OUT_CHARGE_PIN, (state & 0x04) ? LOW : HIGH);
    digitalWrite(OUT_CHARGE_FAN_PIN, (state & 0x08) ? LOW : HIGH);
    digitalWrite(OUT_ALARM_PIN, (state & 0x10) ? LOW : HIGH);
}

// Установка светодиодов состояния.
void SetStateLed(int state)
{
    digitalWrite(LED_NORMAL_PIN, (state & 0x01) ? LOW : HIGH);
    digitalWrite(LED_CONNECT_PIN, (state & 0x02) ? LOW : HIGH);
    digitalWrite(LED_CHARGE_PIN, (state & 0x04) ? LOW : HIGH);
    digitalWrite(LED_CHARGE_FAN_PIN, (state & 0x08) ? LOW : HIGH);
    digitalWrite(LED_ALARM_PIN, (state & 0x10) ? LOW : HIGH);
}

// Установка реле.
void SetRele(int state)
{
    digitalWrite(RELE_1_PIN, (state & 0x01) ? HIGH : LOW);
    digitalWrite(RELE_2_PIN, (state & 0x02) ? HIGH : LOW);
    digitalWrite(RELE_3_PIN, (state & 0x04) ? HIGH : LOW);
}

// Установка светодиодов реле.
void SetReleLed(int state)
{
    digitalWrite(LED_RELE_1_PIN, (state & 0x01) ? LOW : HIGH);
    digitalWrite(LED_RELE_2_PIN, (state & 0x02) ? LOW : HIGH);
    digitalWrite(LED_RELE_3_PIN, (state & 0x04) ? LOW : HIGH);
}

// Установка светодиодов входов.
void SetInLed(int state)
{
    digitalWrite(LED_IN_1_PIN, (state & 0x01) ? LOW : HIGH);
    digitalWrite(LED_IN_2_PIN, (state & 0x02) ? LOW : HIGH);
    digitalWrite(LED_IN_3_PIN, (state & 0x04) ? LOW : HIGH);
    digitalWrite(LED_IN_4_PIN, (state & 0x08) ? LOW : HIGH);
}

// Определение состояния входов.
int GetStateIn()
{
    int i = 0;
    if (digitalRead(IN_1_PIN) == LOW)
        i |= 0x01;
    if (digitalRead(IN_2_PIN) == LOW)
        i |= 0x02;
    if (digitalRead(IN_3_PIN) == LOW)
        i |= 0x04;
    if (digitalRead(IN_4_PIN) == LOW)
        i |= 0x08;
    return i;
}

// Вывести свет.
void putLight(int r, int g, int b, int bright)
{
    pixels.setBrightness(bright);
    pixels.fill(pixels.Color(r, g, b));
    pixels.show();
}

// Определение состояния электромобиля.
STATE_EVSE_E getState()
{
    int min_v = 10000, max_v = -10000;
    int v;
    for (int i = 0; i < 100; i++) {
        v = analogRead(MSR_PWR_PIN);
        if (v < min_v)
            min_v = v;
        if (v > max_v)
            max_v = v;
    }

    // Serial.print("min=");
    // Serial.print(min_v);
    // Serial.print(", max=");
    // Serial.println(max_v);

    if (pwmCur == 0)
        return STATE_NORMAL; // ШИМ выключен.

    if (checkStateA(min_v, max_v))
        return STATE_NORMAL;
    if (checkStateB(min_v, max_v))
        return STATE_CONNECT;
    if (checkStateC(min_v, max_v))
        return STATE_CHARGE;
    if (checkStateD(min_v, max_v))
        return STATE_CHARGE_FAN;

    return STATE_ALARM;
}

// Проверка уровня.
boolean checkLevel(int level, int porog, int delta)
{
    return ((level >= (porog - delta)) && (level <= (porog + delta)));
}

// Проверка уровней минимума и максимума.
boolean checkMinMax(int min_v, int porog_min, int max_v, int porog_max)
{
    return (checkLevel(min_v, porog_min, 50) && checkLevel(max_v, porog_max, 50));
}

boolean checkStateA(int min_v, int max_v)
{
    return (checkMinMax(min_v, 288, max_v, 1023));
}

boolean checkStateB(int min_v, int max_v)
{
    return (checkMinMax(min_v, 288, max_v, 963));
}

boolean checkStateC(int min_v, int max_v)
{
    return (checkMinMax(min_v, 288, max_v, 859));
}

boolean checkStateD(int min_v, int max_v)
{
    return (checkMinMax(min_v, 288, max_v, 764));
}
