
/*
*********************************************************************************************************
*
*                   (c) Copyright 2003-2017, INITEC, Ekaterinburg. Russia.
*                                    All Rights Reserved
*
*
* Project     : Any
*
* Filemane    : Keyb.cpp
*
* Target      : Independent
* For         : ANSI C
*
* Description : Реализация класса клавиатуры.
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
* By          : 01.07.2017
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              INCLUDES
*********************************************************************************************************
*/

  #include <Arduino.h>
  #include "Keyb.h"

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

  // Маски битов.
  #define MASK_BTN_1          0x01  // Маска кнопки 1.
  #define MASK_BTN_2          0x02  // Маска кнопки 2.
  #define MASK_BTN_3          0x04  // Маска кнопки 3.
  #define MASK_BTN_4          0x08  // Маска кнопки 4.

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

  // Конструктор.
  Keyb::Keyb(int pin1, int pin2, int pin3, int pin4)
  {
    inputPin1 = pin1;
    inputPin2 = pin2;
    inputPin3 = pin3;
    inputPin4 = pin4;

    pinMode(inputPin1, INPUT);
    pinMode(inputPin2, INPUT);
    pinMode(inputPin3, INPUT);
    pinMode(inputPin4, INPUT);

    statePin          = getStatePins();
    statePinOld       = 0;
    stateHold         = 0;
    stateHoldOld      = 0;

    event             = 0;
    end_event_flag    = 0;

    longPressFlag     = 0;
  }

  // Определение кода события.
  int Keyb::getEvent() {
    return event;
  }
  
  // Определние флага события.
  int Keyb::getEventFlag() {
    return end_event_flag;
  }
  
  // Сброс флага события.
  int Keyb::resetEventFlag() {
    end_event_flag  = 0;
    event           = 0;
  }
  
  // Квант работы.
  void Keyb::run() {
    if ((millis() - millisInputHoldOld) > TIME_INPUT_READ) {
      inputReadQuant();
      millisInputHoldOld = millis();
    }
  }

/*
*********************************************************************************************************
*                                            LOCAL FUNCTIONS
*********************************************************************************************************
*/

  // Квант опроса входов.
  void Keyb::inputReadQuant() {
    statePin = getStatePins();
  
    if (statePin != statePinOld) {
      // Состояние входов изменилось.
      inputHoldCount = 0;
      statePinOld = statePin;
    }
    else {
      // Состояние входов не изменилось.
      inputHoldCount++;
      if (inputHoldCount >= INPUT_HOLD_COUNT) {
        inputHoldCount = 0;
  
        stateHold = statePin;



        
        
        if (stateHold != stateHoldOld) {
          // Изменилось состояние кнопок.

          if ((stateHold & MASK_BTN_1) && !(stateHoldOld & MASK_BTN_1))  {
            // Нажали кнопку 1.
            event = KEYB_EVENT_PRESS_BTN_1;
            end_event_flag = 1;
          }
          
          if ((stateHold & MASK_BTN_2) && !(stateHoldOld & MASK_BTN_2))  {
            // Нажали кнопку 2.
            event = KEYB_EVENT_PRESS_BTN_2;
            end_event_flag = 1;
          }

          if ((stateHold & MASK_BTN_3) && !(stateHoldOld & MASK_BTN_3))  {
            // Нажали кнопку 3.
            event = KEYB_EVENT_PRESS_BTN_3;
            end_event_flag = 1;
          }

          if ((stateHold & MASK_BTN_4) && !(stateHoldOld & MASK_BTN_4))  {
            // Нажали кнопку 4.
            event = KEYB_EVENT_PRESS_BTN_4;
            end_event_flag = 1;
          }

          
          if (!(stateHold & MASK_BTN_1) && (stateHoldOld & MASK_BTN_1))  {
            // Отжали кнопку 1.
            event = KEYB_EVENT_UN_PRESS_BTN_1;
            end_event_flag = 1;
            longPressFlag  = 0;
          }
          
          if (!(stateHold & MASK_BTN_2) && (stateHoldOld & MASK_BTN_2))  {
            // Отжали кнопку 2.
            event = KEYB_EVENT_UN_PRESS_BTN_2;
            end_event_flag = 1;
            longPressFlag  = 0;
          }

          if (!(stateHold & MASK_BTN_3) && (stateHoldOld & MASK_BTN_3))  {
            // Отжали кнопку 3.
            event = KEYB_EVENT_UN_PRESS_BTN_3;
            end_event_flag = 1;
            longPressFlag  = 0;
          }

          if (!(stateHold & MASK_BTN_4) && (stateHoldOld & MASK_BTN_4))  {
            // Отжали кнопку 3.
            event = KEYB_EVENT_UN_PRESS_BTN_4;
            end_event_flag = 1;
            longPressFlag  = 0;
          }

         
          stateHoldOld = stateHold;
        }
      }    
    }
  }

  // Опрос входов.
  int Keyb::getStatePins() {
    int i = 0;
  
    if (digitalRead(inputPin1)  == 0) i |= MASK_BTN_1;
    if (digitalRead(inputPin2)  == 0) i |= MASK_BTN_2;
    if (digitalRead(inputPin3)  == 0) i |= MASK_BTN_3;
    if (digitalRead(inputPin4)  == 0) i |= MASK_BTN_4;
  
    return i;
  }
