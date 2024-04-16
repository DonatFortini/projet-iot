#include <Servo.h>

Servo servoBot;
Servo servoTop;

const int maxStep = 4;

int data[2] = {90, 90};
int last_data[2] = {90, 90};

void setup()
{
  servoBot.attach(10);
  servoTop.attach(9);

  servoBot.write(data[0]);
  servoTop.write(data[1]);
  Serial.begin(9600);
}

void loop()
{
  receivedData(data);

  if (data[0] != last_data[0] || data[1] != last_data[1])
  {
    servoBot.write(data[0]);
    servoTop.write(data[1]);

    last_data[0] = data[0];
    last_data[1] = data[1];
  }

  delay(100);
}

void receivedData(int *data)
{
  if (Serial.available() > 0)
  {
    char receivedChars[50];
    memset(receivedChars, 0, sizeof(receivedChars));                       
    Serial.readBytesUntil('\n', receivedChars, sizeof(receivedChars) - 1); 
    int number1, number2;
    int fieldsRead = sscanf(receivedChars, "%d,%d", &number1, &number2);

    if (fieldsRead == 2)
    {
      Serial.print("Entier 1: ");
      Serial.println(number1);
      Serial.print("Entier 2: ");
      Serial.println(number2);
      data[0] = number1;
      data[1] = number2;
    }
    else
    {
      Serial.println("Erreur: format non valide. Utilisez le format 'entier1,entier2'.");
    }
  }
}
