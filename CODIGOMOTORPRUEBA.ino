#define IN1 18  // Pin de control 1
#define IN2 19  // Pin de control 2

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
}

void loop() {
  moverAdelante();
  delay(2000);  // 2 segundos hacia adelante

  detenerMotor();
  delay(1000);  // 1 segundo de pausa

  moverAtras();
  delay(2000);  // 2 segundos hacia atrás

  detenerMotor();
  delay(1000);  // 1 segundo de pausa
}

void moverAdelante() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void moverAtras() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

void detenerMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}
