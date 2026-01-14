void setup() {
  Serial.begin(9600);
}

void loop() {
  if(Serial.available()){
    int n = Serial.read();
    Serial.write(n);
  }

}
