String raw = "[1,2,3,4]";

void setup() {
  Serial.begin(9600);
}

void loop() {
  for (int i = 0; i < raw.length(); i++) {
    char character = raw[i];
    if ('[' == character) {
      continue;
    }
    if (',' == character) {
      continue;
    }
    printInt(int(character) - 48);
  }
  Serial.println("End");
  delay(5000);
}

void printInt(int integer) {
  Serial.print("Int: ");
  Serial.println(integer);
}

