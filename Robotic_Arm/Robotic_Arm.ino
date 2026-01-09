#include <Servo.h>

Servo motor0;
int potpin0 = A0;
int val0;

Servo motor1;
int potpin1 = A1;
int val1;

Servo motor2;
int potpin2 = A2;
int val2;

Servo motor3;
int potpin3 = A3;
int val3;

void setup() {
  
  Serial.begin(9600);

  motor0.attach(3);
  motor1.attach(5);
  motor2.attach(6);
  motor3.attach(10);
}

void loop() {

//inicial
motor0.write(63);
motor1.write(118);
motor2.write(120);
motor3.write(145);

delay(2000);

//abrir garra
motor0.write(63);
motor1.write(118);
motor2.write(120);
motor3.write(79);

delay(2000);

//expandir
motor0.write(63);
motor1.write(118);
for(int i=120; i<=150; i++){
motor2.write(i);
delay(30);
}
motor3.write(79);

delay(2000);

//descer
motor0.write(63);
for(int i=118; i<=145; i++){
motor1.write(i);
delay(50);
}
motor2.write(150);
motor3.write(79);

delay(3000);

//pegar
motor0.write(63);
motor1.write(145);
motor2.write(150);
motor3.write(145);

delay(3000);

//encolher
motor0.write(63);
for(int i=145; i>=100; i--){
motor1.write(i);
delay(50);
}
for(int i=150; i>=120; i--){
motor2.write(i);
delay(30);
}
motor3.write(145);

  delay(3000);

//girar
for(int i=63; i>=0; i--){
motor0.write(i);
delay(10);
}
motor1.write(100);
motor2.write(120);
motor3.write(145);

delay(2000);

//expandir
motor0.write(0);
motor1.write(118);
for(int i=120; i<=180; i++){
motor2.write(i);
delay(30);
}
motor3.write(145);
delay(3000);

//soltar 
motor0.write(0);
motor1.write(118);
motor2.write(180);
motor3.write(79);

delay(3000);

//voltar ao inicial
for(int i=0; i<=63; i++){
motor0.write(i);
delay(10);
}
motor1.write(118);
for(int i=180; i>=120; i++){
motor2.write(i);
delay(30);
}
motor3.write(145);
delay(10000);
}
