void setup() {

  // reset Nina
  pinMode(NINA_RESETN, OUTPUT);

  digitalWrite(NINA_RESETN, HIGH);
  delay(100);
  digitalWrite(NINA_RESETN, LOW);
  delay(750);
  
  Serial2.begin(115200);
  Serial.begin(115200);

  Serial.write("Started");
}

void loop() {
  Serial2.write("abcd");
  delay(500);
  Serial2.write("1234");
  delay(500);
}
