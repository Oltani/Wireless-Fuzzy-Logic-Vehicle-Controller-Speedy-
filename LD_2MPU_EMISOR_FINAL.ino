#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
#include <MPU6050_light.h>
#include <Fuzzy.h>

// Sensor 1: eje X (adelante / atrás) en dirección I2C 0x68
MPU6050 mpu1(Wire); 

// Sensor 2: eje Y (izquierda / derecha) en dirección I2C 0x69
MPU6050 mpu2(Wire);  // Inicializar sin la dirección

// MAC del receptor (carrito)
uint8_t broadcastAddress[] = {0xFC, 0xB4, 0x67, 0xF3, 0xFE, 0x94};

// Mensaje que se envía
typedef struct struct_message {
  char accion[10];
} struct_message;

struct_message mensaje;

// Sistema difuso
Fuzzy* fuzzy = new Fuzzy();

// Sets difusos
FuzzySet *ax_neg, *ax_zero, *ax_pos;
FuzzySet *ay_neg, *ay_zero, *ay_pos;
FuzzySet *out_atras, *out_parar, *out_adelante;
FuzzySet *out_izquierda, *out_derecha;

void setupFuzzy() {
  // INPUT: Aceleración en X
  FuzzyInput* accX = new FuzzyInput(1);
  ax_neg = new FuzzySet(-1.2, -0.9, -0.6, -0.3);
  ax_zero = new FuzzySet(-0.3, -0.1, 0.1, 0.3);
  ax_pos = new FuzzySet(0.3, 0.6, 0.9, 1.2);
  accX->addFuzzySet(ax_neg);
  accX->addFuzzySet(ax_zero);
  accX->addFuzzySet(ax_pos);
  fuzzy->addFuzzyInput(accX);

  // INPUT: Aceleración en Y
  FuzzyInput* accY = new FuzzyInput(2);
  ay_neg = new FuzzySet(-1.0, -0.6, -0.5, -0.3);
  ay_zero = new FuzzySet(-0.3, -0.1, 0.1, 0.3);
  ay_pos = new FuzzySet(0.3, 0.5, 0.6, 1.0);
  accY->addFuzzySet(ay_neg);
  accY->addFuzzySet(ay_zero);
  accY->addFuzzySet(ay_pos);
  fuzzy->addFuzzyInput(accY);

  // OUTPUT: Acción
  FuzzyOutput* accion = new FuzzyOutput(1);
  out_atras     = new FuzzySet(0, 0, 0, 0.5);
  out_izquierda = new FuzzySet(0.5, 1, 1, 1.5);
  out_parar     = new FuzzySet(1.5, 2, 2, 2.5);
  out_derecha   = new FuzzySet(2.5, 3, 3, 3.5);
  out_adelante  = new FuzzySet(3.5, 4, 4, 4);
  accion->addFuzzySet(out_atras);
  accion->addFuzzySet(out_izquierda);
  accion->addFuzzySet(out_parar);
  accion->addFuzzySet(out_derecha);
  accion->addFuzzySet(out_adelante);
  fuzzy->addFuzzyOutput(accion);

  // Reglas
  FuzzyRuleAntecedent* r1 = new FuzzyRuleAntecedent();
  r1->joinWithAND(ax_pos, ay_zero); // Regla para "adelante"
  FuzzyRuleConsequent* c1 = new FuzzyRuleConsequent();
  c1->addOutput(out_adelante);
  fuzzy->addFuzzyRule(new FuzzyRule(1, r1, c1));

  FuzzyRuleAntecedent* r2 = new FuzzyRuleAntecedent();
  r2->joinWithAND(ax_neg, ay_zero); // Regla para "atrás"
  FuzzyRuleConsequent* c2 = new FuzzyRuleConsequent();
  c2->addOutput(out_atras);
  fuzzy->addFuzzyRule(new FuzzyRule(2, r2, c2));

  FuzzyRuleAntecedent* r3 = new FuzzyRuleAntecedent();
  r3->joinWithAND(ax_zero, ay_pos); // Regla para "derecha"
  FuzzyRuleConsequent* c3 = new FuzzyRuleConsequent();
  c3->addOutput(out_derecha);
  fuzzy->addFuzzyRule(new FuzzyRule(3, r3, c3));

  FuzzyRuleAntecedent* r4 = new FuzzyRuleAntecedent();
  r4->joinWithAND(ax_zero, ay_neg); // Regla para "izquierda"
  FuzzyRuleConsequent* c4 = new FuzzyRuleConsequent();
  c4->addOutput(out_izquierda);
  fuzzy->addFuzzyRule(new FuzzyRule(4, r4, c4));

  FuzzyRuleAntecedent* r5 = new FuzzyRuleAntecedent();
  r5->joinWithAND(ax_zero, ay_zero); // Regla para "parar"
  FuzzyRuleConsequent* c5 = new FuzzyRuleConsequent();
  c5->addOutput(out_parar);
  fuzzy->addFuzzyRule(new FuzzyRule(5, r5, c5));
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(1000);

  if (mpu1.begin() != 0) {
    Serial.println("Error MPU6050 #1");
    while (1);
  }
  if (mpu2.begin() != 0) {
    Serial.println("Error MPU6050 #2");
    while (1);
  }
  mpu1.calcOffsets();
  mpu2.setAddress(0x69); // Cambiar la dirección del segundo sensor MPU6050
  mpu2.calcOffsets();

  WiFi.mode(WIFI_STA);
  esp_now_init();

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  setupFuzzy();
}

void loop() {
  mpu1.update();  // eje X (adelante/atrás)
  mpu2.update();  // eje Y (izquierda/derecha)

  float ax = mpu1.getAccX();
  float ay = mpu2.getAccY();

  fuzzy->setInput(1, ax);
  fuzzy->setInput(2, ay);
  fuzzy->fuzzify();
  float salida = fuzzy->defuzzify(1);

  int accion = round(salida);
  switch (accion) {
    case 0: strcpy(mensaje.accion, "atras"); break;
    case 1: strcpy(mensaje.accion, "izquierda"); break;
    case 2: strcpy(mensaje.accion, "parar"); break;
    case 3: strcpy(mensaje.accion, "derecha"); break;
    case 4: strcpy(mensaje.accion, "adelante"); break;
    default: strcpy(mensaje.accion, "parar"); break;
  }

  esp_now_send(broadcastAddress, (uint8_t *)&mensaje, sizeof(mensaje));

  Serial.print("AX (frente/atrás): "); Serial.print(ax, 2);
  Serial.print(" | AY (izq/derecha): "); Serial.print(ay, 2);
  Serial.print(" | Acción: "); Serial.println(mensaje.accion);

  delay(200);
}
