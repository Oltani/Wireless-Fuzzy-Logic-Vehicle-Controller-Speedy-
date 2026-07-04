#include <Wire.h>
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // Pines SDA y SCL del ESP32

  byte status = mpu.begin();
  if (status != 0) {
    Serial.print("Error al iniciar MPU6050. Código: ");
    Serial.println(status);
    while (1);
  }

  Serial.println("MPU6050 conectado. Calibrando...");
  delay(1000);
  mpu.calcGyroOffsets();
  Serial.println("Calibrado.");
}

void loop() {
  mpu.update();

  Serial.print("Ángulo X: "); Serial.print(mpu.getAngleX());
  Serial.print(" | Ángulo Y: "); Serial.print(mpu.getAngleY());
  Serial.print(" | Ángulo Z: "); Serial.println(mpu.getAngleZ());

  delay(500);
}
