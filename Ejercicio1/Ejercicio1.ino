#include <U8g2lib.h>
#include <DHT.h>
#include <Preferences.h>
#include <Ticker.h>

// OBJETOS
Preferences preferences;
Ticker tickerBoton;  // temporizador para contar segundos del boton

// SENSOR Y OLED
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// BOTONES
#define sw1 35
#define sw2 34

// ESTADOS
#define PANTALLA_1 1
#define ESPERAR_SW1 2
#define PANTALLA_2 3
#define AUMENTAR_UMBRAL 4
#define DISMINUIR_UMBRAL 5

int estado = PANTALLA_1;

// VARIABLES
float temperatura = 0;
int umbral = 26;
int segundosBoton = 0;  // contador de segundos


void setup() {
  Serial.begin(115200);
  pinMode(sw1, INPUT_PULLUP);
  pinMode(sw2, INPUT_PULLUP);
  dht.begin();
  u8g2.begin();
  // Ticker cada 1s para contar segundos
  tickerBoton.attach(1, tiempoBotones);

  // Leer umbral guardado
  preferences.begin("my-app", false);
  umbral = preferences.getInt("umbral", 26);
  preferences.end();
}


void loop() {
  temperatura = dht.readTemperature();
  switch (estado) {
    case PANTALLA_1:
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      u8g2.drawStr(5, 15, "Temp:");
      char bufferTemp[10];
      sprintf(bufferTemp, "%.1f C", temperatura);
      u8g2.drawStr(50, 15, bufferTemp);

      u8g2.drawStr(5, 35, "Umbral:");
      char bufferUmbralP1[10];
      sprintf(bufferUmbralP1, "%d C", umbral);
      u8g2.drawStr(50, 35, bufferUmbralP1);
      u8g2.sendBuffer();

      if (digitalRead(sw1) == LOW) {
        segundosBoton = 0;
        Serial.println(segundosBoton);
        estado = ESPERAR_SW1;
      }
      break;

    case ESPERAR_SW1:
      if (digitalRead(sw1) == HIGH) {
        estado = PANTALLA_1;
      } else if (segundosBoton >= 5) {
        estado = PANTALLA_2;
      }
      break;

    case PANTALLA_2:
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      u8g2.drawStr(5, 20, "VU:");
      char bufferUmbralP2[10];
      sprintf(bufferUmbralP2, "%d C", umbral);
      u8g2.drawStr(50, 20, bufferUmbralP2);
      u8g2.sendBuffer();

      if (digitalRead(sw1) == LOW) estado = AUMENTAR_UMBRAL;
      else if (digitalRead(sw2) == LOW) estado = DISMINUIR_UMBRAL;
      break;

    case AUMENTAR_UMBRAL:
      if (digitalRead(sw1) == HIGH) {
        umbral++;
        estado = PANTALLA_2;
      }
      break;

    case DISMINUIR_UMBRAL:
      segundosBoton = 0;
      if (digitalRead(sw2) == HIGH) {
        umbral--;
        Serial.println(segundosBoton);
        estado = PANTALLA_2;
      } else if (segundosBoton >= 5) {
        estado = PANTALLA_1;
        preferences.begin("my-app", false);
        preferences.putInt("umbral", umbral);
        preferences.end();
        Serial.println("Umbral guardado en memoria");
      }
      break;
  }
}

// --- FUNCIONES AUXILIARES ---
void tiempoBotones() {
  Serial.println(segundosBoton);
  segundosBoton++;
}
