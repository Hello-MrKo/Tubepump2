// 결선 

//  nodemcu   -   TB6600 (M1),    TB6600(M2)    
//  D1        -     DIR+         
//  D2        -     PUL+
//  GND       -     PUL-     -       PUL-
//  D5                       -       DIR+
//  D6                       -       PUL+





#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <AccelStepper.h>

// Wi-Fi 설정
const char* ssid = "farmersday";                                                //  router  Wifi ID
const char* password = "a88888888";                                            //   router  wifi PW                         

// 웹 서버 포트 80
ESP8266WebServer server(80);

// 첫 번째 모터 설정 (TB6600 드라이버 사용)
#define DIR1_PIN D1  
#define STEP1_PIN D2  

// 두 번째 모터 설정 (TB6600 드라이버 사용)
#define DIR2_PIN D5  
#define STEP2_PIN D6  

// AccelStepper 객체 생성
AccelStepper stepper1(AccelStepper::DRIVER, STEP1_PIN, DIR1_PIN);
AccelStepper stepper2(AccelStepper::DRIVER, STEP2_PIN, DIR2_PIN);

// 모터 상태 변수
bool isRunning1 = false;
bool isRunning2 = false;

// 기본 값 설정
float maxSpeed1 = 1000;  // 첫 번째 모터 기본 최대 속도    (500 ~ 2000)
float acceleration1 = 1000;  // 첫 번째 모터 기본 가속도    (100 ~ 2000)
long position1Increment = 500;  // 첫 번째 모터 상대 이동  (0 ~ 수십만)

float maxSpeed2 = 1000;  // 두 번째 모터 기본 최대 속도     (500 ~ 2000)
float acceleration2 = 1000;  // 두 번째 모터 기본 가속도      (100 ~ 2000)
long position2Increment = 500;  // 두 번째 모터 상대 이동    (0 ~ 수십만)

void handleRoot() {
  String html = "<html>\
  <head>\
    <title>Stepper Motor Control</title>\
  </head>\
  <body>\
    <h1>Stepper Motor Control</h1>\
    <h2>Motor 1 Control</h2>\
    <p>Max Speed: <input type='number' id='maxSpeed1' value='" + String(maxSpeed1) + "'> </p>\
    <p>Acceleration: <input type='number' id='acceleration1' value='" + String(acceleration1) + "'> </p>\
    <p>Position Increment: <input type='number' id='position1Increment' value='" + String(position1Increment) + "'> </p>\
    <button onclick='sendControl1()'>Run Motor 1</button>\
    <button onclick='stopMotor1()'>Stop Motor 1</button>\
    <h2>Motor 2 Control</h2>\
    <p>Max Speed: <input type='number' id='maxSpeed2' value='" + String(maxSpeed2) + "'> </p>\
    <p>Acceleration: <input type='number' id='acceleration2' value='" + String(acceleration2) + "'> </p>\
    <p>Position Increment: <input type='number' id='position2Increment' value='" + String(position2Increment) + "'> </p>\
    <button onclick='sendControl2()'>Run Motor 2</button>\
    <button onclick='stopMotor2()'>Stop Motor 2</button>\
    <script>\
      function sendControl1() {\
        var maxSpeed1 = document.getElementById('maxSpeed1').value;\
        var acceleration1 = document.getElementById('acceleration1').value;\
        var position1Increment = document.getElementById('position1Increment').value;\
        var xhttp = new XMLHttpRequest();\
        xhttp.open('GET', '/control1?maxSpeed1=' + maxSpeed1 + '&acceleration1=' + acceleration1 + '&position1Increment=' + position1Increment, true);\
        xhttp.send();\
      }\
      function stopMotor1() {\
        var xhttp = new XMLHttpRequest();\
        xhttp.open('GET', '/stop1', true);\
        xhttp.send();\
      }\
      function sendControl2() {\
        var maxSpeed2 = document.getElementById('maxSpeed2').value;\
        var acceleration2 = document.getElementById('acceleration2').value;\
        var position2Increment = document.getElementById('position2Increment').value;\
        var xhttp = new XMLHttpRequest();\
        xhttp.open('GET', '/control2?maxSpeed2=' + maxSpeed2 + '&acceleration2=' + acceleration2 + '&position2Increment=' + position2Increment, true);\
        xhttp.send();\
      }\
      function stopMotor2() {\
        var xhttp = new XMLHttpRequest();\
        xhttp.open('GET', '/stop2', true);\
        xhttp.send();\
      }\
    </script>\
  </body>\
  </html>";
  
  server.send(200, "text/html", html);
}

// 첫 번째 모터 제어
void handleControl1() {
  maxSpeed1 = server.arg("maxSpeed1").toFloat();
  acceleration1 = server.arg("acceleration1").toFloat();
  position1Increment = server.arg("position1Increment").toInt();
  
  // 모터에 설정 적용
  stepper1.setMaxSpeed(maxSpeed1);
  stepper1.setAcceleration(acceleration1);
  stepper1.move(position1Increment);  // 상대적으로 position1Increment만큼 이동
  
  isRunning1 = true;

  server.send(200, "text/plain", "Motor 1 running");
}

// 두 번째 모터 제어
void handleControl2() {
  maxSpeed2 = server.arg("maxSpeed2").toFloat();
  acceleration2 = server.arg("acceleration2").toFloat();
  position2Increment = server.arg("position2Increment").toInt();
  
  // 모터에 설정 적용
  stepper2.setMaxSpeed(maxSpeed2);
  stepper2.setAcceleration(acceleration2);
  stepper2.move(position2Increment);  // 상대적으로 position2Increment만큼 이동
  
  isRunning2 = true;

  server.send(200, "text/plain", "Motor 2 running");
}

// 첫 번째 모터 정지
void handleStop1() {
  isRunning1 = false;
  stepper1.stop();
  
  server.send(200, "text/plain", "Motor 1 stopped");
}

// 두 번째 모터 정지
void handleStop2() {
  isRunning2 = false;
  stepper2.stop();
  
  server.send(200, "text/plain", "Motor 2 stopped");
}

void setup() {
  Serial.begin(115200);
  
  // Wi-Fi 연결
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // 웹 서버 경로 설정
  server.on("/", handleRoot);
  server.on("/control1", handleControl1);
  server.on("/control2", handleControl2);
  server.on("/stop1", handleStop1);
  server.on("/stop2", handleStop2);
  
  // 웹 서버 시작
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // 웹 서버 요청 처리
  server.handleClient();
  
  // 각 모터가 실행 중일 때 run 메서드 실행
  if (isRunning1) {
    stepper1.run();
  }
  if (isRunning2) {
    stepper2.run();
  }
}
