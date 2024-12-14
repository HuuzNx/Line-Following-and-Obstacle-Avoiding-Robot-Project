
#include <AFMotor.h> // Thư viện điều khiển động cơ DC
#include <NewPing.h> // Thư viện điều khiển cảm biến siêu âm
#include <Servo.h>   // Thư viện điều khiển servo

// Định nghĩa các chân kết nối với cảm biến siêu âm
#define ECHO_PIN A0          // Chân Echo
#define TRIG_PIN A1          // Chân Trigger

// Cấu hình thông số
#define MAX_DISTANCE 200     // Khoảng cách tối đa cần đo (cm)
#define MAX_SPEED 235        // Tốc độ tối đa của động cơ
#define MAX_SPEED_OFFSET 20  // Hệ số bù tốc độ để cân bằng giữa hai động cơ

// Khởi tạo các động cơ
AF_DCMotor motor1(2);        // Động cơ 1 trên kênh 2
AF_DCMotor motor2(3);        // Động cơ 2 trên kênh 3

// Khởi tạo servo
Servo myservo;

// Khởi tạo cảm biến siêu âm
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
boolean goesForward = false; // Trạng thái di chuyển tiến
int distance = 100;          // Khoảng cách ban đầu
int speedSet = 0;            // Tốc độ hiện tại
char command;                // Biến để lưu lệnh nhận từ Bluetooth

// Biến kiểm soát trạng thái tránh vật cản
int dugme = 1;

void setup() 
{   
    myservo.attach(9);          // Gắn servo vào chân PWM số 9
    myservo.write(90);          // Đặt servo về góc 90 độ (trung lập)
    delay(1000);                // Chờ servo ổn định

    // Đọc khoảng cách ban đầu từ cảm biến siêu âm
    distance = readPing();
    delay(100);
    distance = readPing();
    delay(100);
    distance = readPing();
    delay(100);
    distance = readPing();
    delay(100);

    Serial.begin(9600); // Khởi tạo giao tiếp Serial với tốc độ 9600 baud
}

void loop(){
    long duration, distance;
    while (dugme == 1) // Vòng lặp chính khi chế độ tránh vật cản được bật
    { 
        int distanceR = 0; // Khoảng cách bên phải
        int distanceL = 0; // Khoảng cách bên trái
        delay(40);         // Dừng ngắn để ổn định đọc dữ liệu

        if (distance <= 24) // Nếu khoảng cách nhỏ hơn hoặc bằng 24 cm
        {
            moveStop();           // Dừng xe
            delay(100);
            moveBackward();       // Lùi xe
            delay(300);
            moveStop();           // Dừng xe
            delay(200);

            distanceR = lookRight(); // Đo khoảng cách bên phải
            delay(200);
            distanceL = lookLeft(); // Đo khoảng cách bên trái
            delay(200);

            if (distanceR >= distanceL) // So sánh khoảng cách và chọn hướng
            {
                turnRight();       // Rẽ phải
                moveStop();
            } else {
                turnLeft();        // Rẽ trái
                moveStop();
            }
        } else {
            forward();            // Tiếp tục di chuyển tiến
        }

        distance = readPing();    // Cập nhật khoảng cách mới
    }
}

// Hàm di chuyển tiến
void forward()
{
    motor1.setSpeed(200);         // Cài đặt tốc độ tối đa
    motor1.run(FORWARD);          // Quay động cơ theo chiều kim đồng hồ
    motor2.setSpeed(200);         // Cài đặt tốc độ tối đa
    motor2.run(FORWARD);          // Quay động cơ theo chiều kim đồng hồ
}

// Hàm lùi xe
void back()
{
    motor1.setSpeed(255);         
    motor1.run(BACKWARD);         // Quay động cơ ngược chiều kim đồng hồ
    motor2.setSpeed(255);         
    motor2.run(BACKWARD);         
}

// Hàm xoay trái
void left()
{
    motor1.setSpeed(255);         
    motor1.run(FORWARD);          
    motor2.setSpeed(0);
    motor2.run(RELEASE);          // Dừng động cơ phải
}

// Hàm xoay phải
void right()
{
    motor1.setSpeed(0);
    motor1.run(RELEASE);          // Dừng động cơ trái
    motor2.setSpeed(255);         
    motor2.run(FORWARD);          // Quay động cơ phải
}

// Hàm tiến lệch trái
void aheadLeft()
{
    motor1.setSpeed(255);         // Động cơ trái chạy nhanh
    motor1.run(FORWARD);          
    motor2.setSpeed(125);         // Động cơ phải chạy chậm
    motor2.run(FORWARD);          
}

// Hàm tiến lệch phải
void aheadRight()
{
    motor1.setSpeed(125);         // Động cơ trái chạy chậm
    motor1.run(FORWARD);          
    motor2.setSpeed(255);         // Động cơ phải chạy nhanh
    motor2.run(FORWARD);          
}

// Hàm lùi lệch phải
void backRight()
{
    motor1.setSpeed(125);         
    motor1.run(BACKWARD);
    motor2.setSpeed(255);
    motor2.run(BACKWARD);         
}

// Hàm lùi lệch trái
void backLeft()
{
    motor1.setSpeed(255);
    motor1.run(BACKWARD);         
    motor2.setSpeed(125);         
    motor2.run(BACKWARD);         
}

// Hàm dừng xe
void Stop()
{
    motor1.setSpeed(0);
    motor1.run(RELEASE);          // Dừng động cơ trái
    motor2.setSpeed(0);
    motor2.run(RELEASE);          // Dừng động cơ phải
}

// Hàm dừng chế độ tránh vật cản
void stopAvoiding()
{
    dugme = 0;
    motor1.setSpeed(0);
    motor1.run(RELEASE);          
    motor2.setSpeed(0);
    motor2.run(RELEASE);          
}

// Hàm xoay servo để nhìn bên phải và đo khoảng cách
int lookRight()
{
    myservo.write(50);            // Xoay servo sang phải
    delay(200);
    int distance = readPing();    // Đo khoảng cách
    delay(100);
    myservo.write(105);           // Trả servo về trung lập
    return distance;
}

// Hàm xoay servo để nhìn bên trái và đo khoảng cách
int lookLeft()
{
    myservo.write(160);           // Xoay servo sang trái
    delay(200);
    int distance = readPing();    // Đo khoảng cách
    delay(100);
    myservo.write(105);           // Trả servo về trung lập
    return distance;
}

// Hàm đọc khoảng cách từ cảm biến siêu âm
int readPing() { 
    delay(70);
    int cm = sonar.ping_cm();     // Đọc khoảng cách và chuyển đổi sang cm
    if (cm == 0)                  // Nếu không đo được khoảng cách
    {
        cm = 250;                 // Gán giá trị lớn hơn khoảng cách tối đa
    }
    return cm;
}

// Hàm dừng xe
void moveStop() {
    motor1.run(RELEASE);          // Dừng động cơ trái
    motor2.run(RELEASE);          // Dừng động cơ phải
} 

// Hàm di chuyển tiến với tăng tốc
void moveForward() {
    if (!goesForward)
    {
        goesForward = true;
        motor1.run(FORWARD);      
        motor2.run(FORWARD);      
        for (speedSet = 0; speedSet < MAX_SPEED; speedSet += 2) 
        {
            motor1.setSpeed(speedSet);
            motor2.setSpeed(speedSet + MAX_SPEED_OFFSET);
            delay(5);
        }
    }
}

// Hàm lùi xe với tăng tốc
void moveBackward() {
    goesForward = false;
    motor1.run(BACKWARD);         
    motor2.run(BACKWARD);         
    for (speedSet = 0; speedSet < MAX_SPEED; speedSet += 2) 
    {
        motor1.setSpeed(speedSet);
        motor2.setSpeed(speedSet + MAX_SPEED_OFFSET);
        delay(5);
    }
}

// Hàm xoay phải tại chỗ
void turnRight() {
    motor2.run(FORWARD);          // Động cơ phải chạy tiến
    motor1.run(BACKWARD);         // Động cơ trái chạy lùi
    delay(300);
    motor2.run(FORWARD);          // Cả hai động cơ chạy tiến
    motor1.run(FORWARD);          
} 

// Hàm xoay trái tại chỗ
void turnLeft() {
    motor2.run(BACKWARD);         // Động cơ phải chạy lùi
    motor1.run(FORWARD);          // Động cơ trái chạy tiến
    delay(300);
    motor2.run(FORWARD);          // Cả hai động cơ chạy tiến
    motor1.run(FORWARD);
}
