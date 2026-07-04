#include <esp_now.h>
#include <WiFi.h>

typedef struct struct_message {
  char accion[10];
} struct_message;

struct_message mensaje;

#define M1A 33
#define M1B 32
#define M2A 25
#define M2B 26
#define M3A 5
#define M3B 4
#define M4A 19
#define M4B 18

void moverAdelante() {
  digitalWrite(M1A, HIGH); digitalWrite(M1B, LOW);
  digitalWrite(M2A, HIGH); digitalWrite(M2B, LOW);
  digitalWrite(M3A, HIGH); digitalWrite(M3B, LOW);
  digitalWrite(M4A, HIGH); digitalWrite(M4B, LOW);
}

void moverAtras() {
  digitalWrite(M1A, LOW); digitalWrite(M1B, HIGH);
  digitalWrite(M2A, LOW); digitalWrite(M2B, HIGH);
  digitalWrite(M3A, LOW); digitalWrite(M3B, HIGH);
  digitalWrite(M4A, LOW); digitalWrite(M4B, HIGH);
}

void girarIzquierda() {
  digitalWrite(M1A, LOW); digitalWrite(M1B, LOW);
  digitalWrite(M2A, HIGH); digitalWrite(M2B, LOW);
  digitalWrite(M3A, LOW); digitalWrite(M3B, LOW);
  digitalWrite(M4A, HIGH); digitalWrite(M4B, LOW);
}

void girarDerecha() {
  digitalWrite(M1A, HIGH); digitalWrite(M1B, LOW);
  digitalWrite(M2A, LOW); digitalWrite(M2B, LOW);
  digitalWrite(M3A, HIGH); digitalWrite(M3B, LOW);
  digitalWrite(M4A, LOW); digitalWrite(M4B, LOW);
}

void pararMotores() {
  digitalWrite(M1A, LOW); digitalWrite(M1B, LOW);
  digitalWrite(M2A, LOW); digitalWrite(M2B, LOW);
  digitalWrite(M3A, LOW); digitalWrite(M3B, LOW);
  digitalWrite(M4A, LOW); digitalWrite(M4B, LOW);
}

// Esta función es llamada cuando el receptor recibe un mensaje
void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  // Verificar si el tamaño de los datos es correcto antes de copiar
  if (len == sizeof(mensaje)) {
    memcpy(&mensaje, data, sizeof(mensaje));
    Serial.print("Mensaje recibido: ");
    Serial.println(mensaje.accion);  // Imprime la acción recibida

    if (strcmp(mensaje.accion, "adelante") == 0) {
      moverAdelante();
    } else if (strcmp(mensaje.accion, "atras") == 0) {
      moverAtras();
    } else if (strcmp(mensaje.accion, "izquierda") == 0) {
      girarIzquierda();
    } else if (strcmp(mensaje.accion, "derecha") == 0) {
      girarDerecha();
    } else {
      pararMotores();
    }
  } else {
    Serial.println("Tamaño de datos incorrecto");
  }

  delay(50);  // Pequeño retraso para evitar sobrecarga
}

void setup() {
  Serial.begin(115200);  // Inicia la comunicación serial para ver el monitor serial
  WiFi.mode(WIFI_STA);

  int pinesMotores[] = {M1A, M1B, M2A, M2B, M3A, M3B, M4A, M4B};
  for (int i = 0; i < 8; i++) {
    pinMode(pinesMotores[i], OUTPUT);
    digitalWrite(pinesMotores[i], LOW);
  }

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al iniciar ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onReceive);
}

void loop() {
  // Nada que hacer aquí
}
