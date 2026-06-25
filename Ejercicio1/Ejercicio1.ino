// CHesler, Sher, Szew, Tovorovsky - Grupo 7
#include <U8g2lib.h>
#include <DHT.h>
#include <Preferences.h>
#include <Ticker.h>

// OBJETOS
Preferences preferences;
Ticker tickerBoton;

// SENSOR Y OLED
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// BOTONES
#define sw1 35
#define sw2 34

// ESTADOS ORDENADOS LOGICAMENTE
#define PANTALLA_1 1
#define SALIR_PANTALLA_1 2
#define PANTALLA_2 3
#define AUMENTAR_UMBRAL 4
#define DISMINUIR_UMBRAL 5
#define SALIR_PANTALLA_2 6

int estado = PANTALLA_1;

// VARIABLES
float temperatura = 0;
int umbral = 26;
volatile int segundosBoton = 0;


void setup() {
  Serial.begin(115200);
  pinMode(sw1, INPUT_PULLUP);
  pinMode(sw2, INPUT_PULLUP);

  dht.begin();
  u8g2.begin();

  // Ticker cada 1s para contar segundos
  tickerBoton.attach(1, tiempoBotones);

  // Leer umbral guardado al encender
  preferences.begin("my-app", false);         //en false para poder leer y escribir, true es solo leer
  umbral = preferences.getInt("umbral", 26);  //buscamos el dato umbral(declarado abajo de todo), si no esta, la variable umbral vale 26
  preferences.end();
}


void loop() {
  temperatura = dht.readTemperature();
  switch (estado) {
    // ---------------- PANTALLA 1 ----------------
    case PANTALLA_1:
      Serial.println("Pantalla 1");
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);

      u8g2.drawStr(5, 15, "VA:");
      char mensajeTemp[10];
      sprintf(mensajeTemp, "%.1f C", temperatura);
      u8g2.drawStr(50, 15, mensajeTemp);

      u8g2.drawStr(5, 35, "VU:");
      char mensajeUmbral1[10];
      sprintf(mensajeUmbral1, "%d C", umbral);
      u8g2.drawStr(50, 35, mensajeUmbral1);
      u8g2.sendBuffer();
      // Si tocan sw1, reiniciamos el reloj y vamos a ver si es para salir
      if (digitalRead(sw1) == LOW) {
        segundosBoton = 0;
        estado = SALIR_PANTALLA_1;
      }
      break;

    case SALIR_PANTALLA_1:
      Serial.println("Salir pantalla 1");
      Serial.println(segundosBoton);
      // Espera a que suelte sw1 para tomar la decisión
      if (digitalRead(sw1) == HIGH) {  //no se hace if con doble condicion porque sino va al else de una
        if (segundosBoton >= 5) {
          estado = PANTALLA_2;  // Pasaron los 5 seg, pasamos a pantalla 2
        } else {
          estado = PANTALLA_1;  //no pasaron 5 seg, vuelve a pantalla 1
        }
      }
      break;

    // ---------------- PANTALLA 2 ----------------
    case PANTALLA_2:
      Serial.println("Pantalla 2");
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      u8g2.drawStr(5, 20, "VU:");
      char mensajeUmbral2[10];
      sprintf(mensajeUmbral2, "%d C", umbral);
      u8g2.drawStr(50, 20, mensajeUmbral2);
      u8g2.sendBuffer();

      // Derivamos según el botón presionado
      if (digitalRead(sw1) == LOW) {
        estado = AUMENTAR_UMBRAL;
      } else if (digitalRead(sw2) == LOW) {
        segundosBoton = 0;
        estado = DISMINUIR_UMBRAL;
      }
      break;

    // ---------------- ACCIONES ----------------
    case AUMENTAR_UMBRAL:
      Serial.println("Aumentar umbral");
      // Se queda acá hasta que sueltes el botón sw1
      if (digitalRead(sw1) == HIGH) {
        umbral++;
        estado = PANTALLA_2;
      }
      break;

    case DISMINUIR_UMBRAL:
      Serial.println("Disminuir umbral");
      Serial.println(segundosBoton);
      // Se queda acá hasta que sueltes el botón sw2.
      // Al soltar, mira cuánto tiempo pasó.
      if (digitalRead(sw2) == HIGH) {
        if (segundosBoton >= 5) {
          estado = SALIR_PANTALLA_2;  // Fueron 5 seg, toca salir
        } else {
          umbral--;  // Fue rápido, toca restar
          estado = PANTALLA_2;
        }
      }
      break;

    case SALIR_PANTALLA_2:
      Serial.println("Salir pantalla 2 guardar umbral");
      // Solo nos ocupamos de guardar y cambiar de pantalla
      preferences.begin("my-app", false);
      preferences.putInt("umbral", umbral);  //guardamos el dato umbral con un valor de lo que valga la variable umbral
      preferences.end();
      Serial.println("Umbral guardado en memoria!");

      estado = PANTALLA_1;
      break;
  }
}

// --- FUNCIONES AUXILIARES ---
void tiempoBotones() {
  segundosBoton++;
}