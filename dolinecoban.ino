#include <AFMotor.h>

// Định nghĩa động cơ
AF_DCMotor motor1(2); // Động cơ phải, kết nối với chân 2 của L293D
AF_DCMotor motor2(3); // Động cơ trái, kết nối với chân 3 của L293D

// Định nghĩa các chân cảm biến đường
const int SensorLeft   = A5; // Cảm biến trái
const int SensorMiddle = A4; // Cảm biến giữa
const int SensorRight  = A3; // Cảm biến phải

// Biến trạng thái cảm biến
int SL, SM, SR;

// Thời gian kiểm tra để tránh dùng delay()
unsigned long previousMillis = 0;
const long interval = 28.5; // Thời gian kiểm tra mỗi 30ms

// Thời gian đợi trước khi dừng nếu không phát hiện đường
unsigned long lastSensorTime = 0;  // Thời gian cuối cùng cảm biến có tín hiệu
const long sensorTimeout = 3000;   // Thời gian chờ đợi 3 giây (3000ms)

void setup() {
  // Cấu hình các chân cảm biến
  pinMode(SensorLeft, INPUT);
  pinMode(SensorMiddle, INPUT);
  pinMode(SensorRight, INPUT);

  // Khởi động Serial Monitor để theo dõi tín hiệu cảm biến
  Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();

  // Kiểm tra xem thời gian kiểm tra đã đủ chưa
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Đọc trạng thái các cảm biến
    SL = digitalRead(SensorLeft);
    SM = digitalRead(SensorMiddle);
    SR = digitalRead(SensorRight);

    // Hiển thị trạng thái các cảm biến lên Serial Monitor để theo dõi
    Serial.print("SL: "); Serial.print(SL);
    Serial.print(" | SM: "); Serial.print(SM);
    Serial.print(" | SR: "); Serial.println(SR);

    // Cập nhật thời gian nếu có tín hiệu từ cảm biến
    if (SM == HIGH || SL == HIGH || SR == HIGH) {
      lastSensorTime = currentMillis; // Cập nhật thời gian có tín hiệu
    }

    // Điều khiển di chuyển dựa trên tín hiệu cảm biến
    if (SM == HIGH) {
      forward(141, 141);  // Tiến thẳng
    }
    else if ((SM == HIGH && SR == HIGH)) {
      turnLeft();  // Cua trái
    }
    else if ((SL == HIGH && SM == HIGH)) {
      turnRight();  // Cua phải
    }
    else if (SL == HIGH && SM == LOW) {
      adjustLeft();  // Điều chỉnh nếu chỉ có cảm biến trái phát hiện line
    }
    else if (SR == HIGH && SM == LOW) {
      adjustRight();  // Điều chỉnh nếu chỉ có cảm biến phải phát hiện line
    }
    else {
      handleNoLine(currentMillis);  // Không có tín hiệu, xử lý dừng xe
    }
  }
}

void forward(int speedLeft, int speedRight) {
  motor1.setSpeed(speedRight);  // Tốc độ động cơ phải
  motor1.run(FORWARD); // Quay động cơ phải
  motor2.setSpeed(speedLeft);  // Tốc độ động cơ trái
  motor2.run(FORWARD); // Quay động cơ trái
}

void stopMotors() {
  motor1.setSpeed(0);  // Dừng động cơ phải
  motor1.run(RELEASE); // Giải phóng động cơ phải
  motor2.setSpeed(0);  // Dừng động cơ trái
  motor2.run(RELEASE); // Giải phóng động cơ trái
}

void turnLeft() {
  motor2.setSpeed(0); // Dừng động cơ trái
  motor2.run(RELEASE); // Giải phóng động cơ trái
  motor1.setSpeed(198); // Quay động cơ phải nhanh hơn
  motor1.run(FORWARD);  // Quay động cơ phải
  delay(520); // Điều chỉnh thời gian quay
  checkMiddleSensor(); // Kiểm tra lại tín hiệu cảm biến giữa
}

void turnRight() {
  motor1.setSpeed(0);  // Dừng động cơ phải
  motor1.run(RELEASE); // Giải phóng động cơ phải
  motor2.setSpeed(198); // Quay động cơ trái nhanh hơn
  motor2.run(FORWARD);  // Quay động cơ trái
  delay(500); // Điều chỉnh thời gian quay
  checkMiddleSensor(); // Kiểm tra lại tín hiệu cảm biến giữa
}

void adjustLeft() {
  motor2.setSpeed(205);  // Tăng tốc động cơ trái
  motor2.run(FORWARD);   // Quay động cơ trái
  motor1.setSpeed(105);  // Giảm tốc động cơ phải
  motor1.run(BACKWARD);  // Quay động cơ phải
  delay(210);            // Giữ trong 210ms để xe dần dần điều chỉnh lại
  checkMiddleSensor();   // Kiểm tra lại cảm biến giữa
}

void adjustRight() {
  motor1.setSpeed(205);  // Tăng tốc động cơ phải
  motor1.run(FORWARD);   // Quay động cơ phải
  motor2.setSpeed(106);  // Giảm tốc động cơ trái
  motor2.run(BACKWARD);  // Quay động cơ trái
  delay(210);            // Giữ trong 210ms để xe dần dần điều chỉnh lại
  checkMiddleSensor();   // Kiểm tra lại cảm biến giữa
}

void checkMiddleSensor() {
  SM = digitalRead(SensorMiddle); 
  if (SM == HIGH) {
    forward(150, 150);  // Tiếp tục đi thẳng
  }
  else {
    adjustDirection();  // Điều chỉnh lại hướng nếu không có tín hiệu giữa
  }
}

void adjustDirection() {
  motor1.setSpeed(130);  // Cập nhật tốc độ động cơ phải
  motor1.run(FORWARD);  // Quay động cơ phải
  motor2.setSpeed(130);  // Cập nhật tốc độ động cơ trái
  motor2.run(FORWARD);  // Quay động cơ trái
}

void handleNoLine(unsigned long currentMillis) {
  // Nếu không có tín hiệu từ cảm biến, kiểm tra xem có nên dừng hay không
  if (currentMillis - lastSensorTime > sensorTimeout) {
    stopMotors();  // Nếu quá 3 giây mà không có tín hiệu, dừng lại
  }
  else {
    // Nếu chưa có tín hiệu từ cảm biến giữa, vẫn di chuyển dựa trên cảm biến ngoài
    if (SL == HIGH) {
      goLeft(160, 160);  // Quẹo trái nhẹ
    }
    else if (SR == HIGH) {
      goRight(161, 161); // Quẹo phải nhẹ
    }
    else {
      forward(150, 150); // Tiếp tục đi thẳng nếu cảm biến ngoài phát hiện line
    }
  }
}

void goLeft(int speedLeft, int speedRight) {
  motor1.setSpeed(0); // Dừng động cơ phải
  motor1.run(RELEASE); // Giải phóng động cơ phải
  motor2.setSpeed(speedRight);  // Quay động cơ trái
  motor2.run(FORWARD); // Quay động cơ trái
}

void goRight(int speedLeft, int speedRight) {
  motor2.setSpeed(0); // Dừng động cơ trái
  motor2.run(RELEASE); // Giải phóng động cơ trái
  motor1.setSpeed(speedRight);  // Quay động cơ phải
  motor1.run(FORWARD); // Quay động cơ phải
}
