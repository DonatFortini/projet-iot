#include <Servo.h>
#include <ezButton.h>

#define SERVO_BOT 10
#define SERVO_TOP 9

#define VRX_PIN A0
#define VRY_PIN A1
#define SW_PIN 3

Servo servoBot;
Servo servoTop;
ezButton button(SW_PIN);

int data[2] = {90, 90};
int last_data[2] = {90, 90};

int xValue = 0;
int yValue = 0;
int buttonState = 0;

bool joystickMode = false;

void receivedData(int *data);
void joystickControl();

void setup()
{
  servoBot.attach(SERVO_BOT);
  servoTop.attach(SERVO_TOP);
  button.setDebounceTime(50);
  servoBot.write(data[0]);
  servoTop.write(data[1]);
  Serial.begin(115200);
}

void loop()
{
  button.loop();
  buttonState = button.getState();

  if (button.isPressed())
  {
    joystickMode = !joystickMode;
    delay(50);
  }

  if (joystickMode)
    joystickControl();
  else
  {
    receivedData(data);
    if (data[0] != last_data[0] || data[1] != last_data[1])
    {
      servoBot.write(data[0]);
      servoTop.write(data[1]);

      last_data[0] = data[0];
      last_data[1] = data[1];
    }
  }
}

/**
 * @brief      Controle de la position des servomoteurs avec un joystick
 */
void joystickControl()
{
  int xRaw = analogRead(VRX_PIN);
  int yRaw = analogRead(VRY_PIN);

  int xMapped = map(xRaw, 0, 1023, 0, 180);
  int yMapped = map(yRaw, 0, 1023, 0, 180);

  servoTop.write(xMapped);
  servoBot.write(yMapped);
}

/**
 * @brief      Reception des données envoyées par le moniteur série
 *
 * @param      data  Tableau de deux entiers
 */
void receivedData(int *data)
{
  if (Serial.available() > 0)
  {
    char receivedChars[50];
    memset(receivedChars, 0, sizeof(receivedChars));
    Serial.readBytesUntil('\n', receivedChars, sizeof(receivedChars) - 1);
    int number1, number2;
    int fieldsRead = sscanf(receivedChars, "/%d,%d", &number1, &number2);

    if (fieldsRead == 2)
    {
      Serial.print("Entier 1: ");
      Serial.println(number1);
      Serial.print("Entier 2: ");
      Serial.println(number2);
      data[0] = number1;
      data[1] = number2;
    }
  }
}
