/**
   Arduino Calculator

   Copyright (C) 2020, Uri Shaked.
   Released under the MIT License.
*/

/*Integrantes:

  Julian Rojas    julianrojas@unicauca.edu.co
  David Cardenas  jcardenass@unicauca.edu.co
  Andrés Sandino  asandino@unicauca.edu.co

*/ 

#include "pinout.h"
#include <Servo.h>
#include "ConstumChar.h"

void showSpalshScreen();
void updateCursor();
/********************************************//**
    Temperature sensor control functions
 ***********************************************/
#include "DHTStable.h"
DHTStable DHT;
/********************************************//**
    Keypad control functions
 ***********************************************/
#include <Keypad.h>
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {5, 4, 3, 2};
byte colPins[KEYPAD_COLS] = {A3, A2, A1, A4};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'.', '0', '=', 'D'}
};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);
/********************************************//**
    LCD control functions
 ***********************************************/
#include <LiquidCrystal.h>
#define DEBUG(a) Serial.print(millis()); Serial.print(": "); Serial.println(a);
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

/********************************************//**
 *  Asynchronous Task control functions
 ***********************************************/
#include <AsyncTaskLib.h>
void mostrarTemp();
void mostrarLuz();

AsyncTask asyncTask1(2000, true,  mostrarTemp );
AsyncTask asyncTask2(1000, true, mostrarLuz );

/********************************************//**
    Sound control functions
 ***********************************************/
#include "Sounds.h"
void sonidoEntrar();
void sonidoErrado();
void sonidoBloquear();

/********************************************//**
    Password control functions
 ***********************************************/
void sistemaContrasenia();
boolean ban = false;
char passwordCorrecta[] = "12345";
char password[6];
byte intentos = 3;
/********************************************//**
    Define global variables
 ***********************************************/
uint64_t value = 0;
unsigned char i = 0;


void setup() {
  lcd.createChar(0, Alien);
  lcd.createChar(1, Block);
  lcd.createChar(2, Unblock);
  lcd.createChar(3, Wrong);
  Serial.begin(115200);
  lcd.begin(16, 2);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER, OUTPUT);	// pin 8 como salida
  delay(10);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);
  asyncTask1.Start();
  asyncTask2.Start();
  showSpalshScreen();
  lcd.clear();
  lcd.cursor();
  lcd.print("Ingrese clave");
  lcd.setCursor(0, 1);
}

void loop() {
  updateCursor();
  char key = keypad.getKey();

  if (!ban) {
    if (key) {
      password[i++] = key;

      lcd.print('*');
    }

    if (i == 5) {
      sistemaContrasenia();
      i = 0;
    }

    if (strcmp(password, passwordCorrecta) == 0) {
      delay(200);
      asyncTask1.Update();
      asyncTask2.Update();
    }
  }
}
/*F**************************************************************************
  NAME: showSpalshScreen
  ----------------------------------------------------------------------------
  PARAMS:   none
  return:   none
  ----------------------------------------------------------------------------
  PURPOSE:
  Displays a customized splash screen on an LCD display
  ----------------------------------------------------------------------------
  NOTE:

*****************************************************************************/
void showSpalshScreen() {
  lcd.print("Bienvenido");
  lcd.setCursor(11, 0);
  lcd.write(byte(0));
  lcd.setCursor(1, 1);
  String message = "Sistema Pro";
  for (byte i = 0; i < message.length(); i++) {
    lcd.print(message[i]);
    delay(50);
  }
  delay(500);
}
/*F**************************************************************************
  NAME: updateCursor
  ----------------------------------------------------------------------------
  PARAMS:   none
  return:   none
  ----------------------------------------------------------------------------
  PURPOSE:
  Toggles the cursor on an LCD display based on a time-based condition
  ----------------------------------------------------------------------------
  NOTE:

*****************************************************************************/
void updateCursor() {
  if (millis() / 250 % 2 == 0 ) {
    lcd.cursor();
  } else {
    lcd.noCursor();
  }
}
/*F**************************************************************************
  NAME: sonidoEntrar
  ----------------------------------------------------------------------------
  PARAMS:   none
  return:   none
  ----------------------------------------------------------------------------
  PURPOSE:
  Plays a sequence of tones on a buzzer to create a sound effect
  ----------------------------------------------------------------------------
  NOTE:

*****************************************************************************/
void sonidoEntrar() {

  for (int i = 0; i < 25; i++) {			// bucle repite 25 veces
    int duracion = 1000 / duraciones[i];		// duracion de la nota en milisegundos
    tone(BUZZER, melodia[i], duracion);	// ejecuta el tono con la duracion
    int pausa = duracion * 1.30;			// calcula pausa
    delay(pausa);					// demora con valor de pausa
    noTone(BUZZER);

  }
}
/*F**************************************************************************
  NAME: sonidoErrado
  ----------------------------------------------------------------------------
  PARAMS:   none
  return:   none
  ----------------------------------------------------------------------------
  PURPOSE:
  Plays an error sound on a buzzer
  ----------------------------------------------------------------------------
  NOTE:

*****************************************************************************/
void sonidoErrado() {
  tone(BUZZER, 1000, 100);
  delay(100);
  noTone(BUZZER);
}
/*F**************************************************************************
  NAME: sonidoBloqueado
  ----------------------------------------------------------------------------
  PARAMS:   none
  return:   none
  ----------------------------------------------------------------------------
  PURPOSE:
  Plays a sequence of tones on a buzzer to create a "blocked" sound effect
  ----------------------------------------------------------------------------
  NOTE:

*****************************************************************************/
void sonidoBloqueado() {

  for (int i = 0; i < 5; i++) {			// bucle repite 25 veces
    int duracion = 1000 / duraciones[i];		// duracion de la nota en milisegundos
    tone(BUZZER, 400, duracion);	// ejecuta el tono con la duracion
    int pausa = duracion * 1.30;			// calcula pausa
    delay(pausa);					// demora con valor de pausa
    noTone(BUZZER);
  }
}
/*F**************************************************************************
  NAME: mostrarTemp
  ----------------------------------------------------------------------------
  PARAMS:   nonce
  return:   none
  ----------------------------------------------------------------------------
  PURPOSE:
  Reads humidity and temperature data from a DHT11 sensor and displays it on both the Serial monitor and an LCD display
  ----------------------------------------------------------------------------
  NOTE:

*****************************************************************************/
void mostrarTemp() {
  Serial.println("nivel de humedad - temperatura");
  Serial.print("DHT11, \t");
  int chk = DHT.read11(DHT11_PIN);
  switch (chk)
  {
    case DHTLIB_OK:
      Serial.print("OK,\t");
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.print("Checksum error,\t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.print("Time out error,\t");
      break;
    default:
      Serial.print("Unknown error,\t");
      break;
  }
  Serial.print(DHT.getHumidity(), 1);
  lcd.setCursor(0, 0);
  lcd.print("H");
  lcd.setCursor(1, 0);
  lcd.print(DHT.getHumidity(), 1);
  lcd.setCursor(0, 1);
  lcd.print("T");
  lcd.setCursor(1, 1);
  Serial.print(",\t");
  Serial.println(DHT.getTemperature(), 1);
  lcd.print(DHT.getTemperature(), 1);
}
/*F**************************************************************************
  NAME: mostrarLuz
  ----------------------------------------------------------------------------
  PARAMS:   none
  return:   none
  ----------------------------------------------------------------------------
  PURPOSE:
  Reads the light level from a photocell and displays it on both the Serial monitor and an LCD display
  ----------------------------------------------------------------------------
  NOTE:

*****************************************************************************/
void mostrarLuz(void) {
  int outputValue = 0;
  Serial.println("nivel de luz");
  outputValue = analogRead(photocellPin);
  lcd.setCursor(6, 0);
  lcd.print("L");
  lcd.setCursor(7, 0);
  lcd.print(outputValue);//print the temperature on lcd1602
  Serial.println(outputValue);
  lcd.setCursor(11, 0);
  lcd.print("");
}
/*F**************************************************************************
  NAME: sistemaContrasenia
  ----------------------------------------------------------------------------
  PARAMS:   none
  return:   none
  ----------------------------------------------------------------------------
  PURPOSE:
  Implements a password-based system to control access. It compares the entered password with the correct password and performs actions accordingly
  ----------------------------------------------------------------------------
  NOTE:

*****************************************************************************/
void sistemaContrasenia() {
  if (strcmp(password, passwordCorrecta) == 0) {
    lcd.clear();
    delay(500);
    lcd.print("Clave Correcta");
    lcd.setCursor(7, 1);
    lcd.write(byte(2));
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, HIGH);
    sonidoEntrar();
    lcd.clear();

    asyncTask1.Start();
    asyncTask2.Start();
  }  else {
    lcd.clear();
    delay(500);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_RED, HIGH);
    sonidoErrado();
    lcd.print("Clave Incorrecta");
    lcd.setCursor(7, 1);
    lcd.write(byte(3));
    delay(2000);
    lcd.clear();
    lcd.print("Ingrese clave");
    lcd.setCursor(0, 1);
    intentos--;
  }

  if (intentos == 0) {
    ban = true;
    lcd.clear();
    delay(10);
    lcd.print("SistemaBloqueado");
    lcd.setCursor(7, 1);
    lcd.write(byte(1));
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_BLUE, HIGH);
    sonidoBloqueado();
  }

  digitalWrite(LED_RED, LOW);
  //digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, LOW);
}

