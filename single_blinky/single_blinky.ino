void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
  pinMode(4, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(13, HIGH);
  digitalWrite(4, LOW);
  delay(250);
  digitalWrite(13, LOW);
  digitalWrite(4, HIGH);
  delay(250);
}
