#include <Wire.h>
#include <EEPROM.h>
byte settings[10];

#define RASPI_POWER_PIN 5
#define RASPI_CAPACITOR_PROBE_PIN A6
#define RASPI_EXTERNAL_POWER_PROBE_PIN 6
#define RASPI_STATE_PIN 16
#define RASPI_SHUTDOWN_PIN 17

#define SETTINGS_IS_INITIALIZED settings[0]
#define SETTINGS_I2C_ADDRESS settings[1]
#define SETTINGS_TURN_OFF_TIMEOUT settings[2]
#define SETTINGS_TURN_OFF_THRESHOLD settings[3]
#define SETTINGS_TURN_ON_THRESHOLD settings[4]

byte i2cRegister = 0;
byte i2cValue = 0;
bool isShutdownPending = false;
bool isShutdownTimerActive = true;
bool isStartupPending = false;
byte shutdownTimerSeconds = 0;
byte debug = 0;

void setup()
{
  
  Serial.begin(115200);

  loadSettings();

  Serial.println("RASPI SUPERCAP v.1.0.1");
  Serial.print("Is initialized: "); Serial.println(SETTINGS_IS_INITIALIZED);
  Serial.print("I2C address: "); Serial.println(SETTINGS_I2C_ADDRESS);
  Serial.print("Turn off timeout (s): "); Serial.println(SETTINGS_TURN_OFF_TIMEOUT);
  Serial.print("Turn off threshold (%): "); Serial.println(SETTINGS_TURN_OFF_THRESHOLD);
  Serial.print("Turn on threshold (%): "); Serial.println(SETTINGS_TURN_ON_THRESHOLD);


  Wire.begin(SETTINGS_I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

 
  pinMode(RASPI_STATE_PIN, INPUT);
  pinMode(RASPI_CAPACITOR_PROBE_PIN, INPUT);
  pinMode(RASPI_EXTERNAL_POWER_PROBE_PIN, INPUT);

  digitalWrite(RASPI_SHUTDOWN_PIN, HIGH);
  pinMode(RASPI_SHUTDOWN_PIN, OUTPUT);

  digitalWrite(RASPI_POWER_PIN, HIGH);
  pinMode(RASPI_POWER_PIN, OUTPUT);
 
}

void loop()
{
  delay(1000);

  diag();
  doLogic();
  Serial.print("  debug : "); Serial.println(debug);
  

  if (i2cRegister == 0x72 && i2cValue == 2)
  {
      i2cValue = 0;
      shutdownPi();
  }
}

void doLogic()
{

  if (hasExternalPower())
  {
    debug = 50;

    if (piState())
    {
      debug = 51;

      isStartupPending = false;

      if (capCharge() <= SETTINGS_TURN_OFF_THRESHOLD && !isShutdownPending && !isStartupPending)
      {
        debug = 52;
        shutdownPi();
      }
    }
    else
    {
      if (isShutdownPending)
      {
        DepowerPi();
        return;
      }
      
      debug = 55;
      
      if (capCharge() >= SETTINGS_TURN_ON_THRESHOLD && !isStartupPending)
      {
        debug = 56;
        turnOnPi();
      }
    }
  }
  else
  {
    debug = 00;

    if (piState())
    {
      debug = 1;

      if (!isShutdownPending)
      {
        debug = 2;

        if (capCharge() <= SETTINGS_TURN_OFF_THRESHOLD)
        {
          debug = 3;

          shutdownPi();
          return;
        }

        if (SETTINGS_TURN_OFF_TIMEOUT < 255)
        {
          debug = 4;

          if (!isShutdownTimerActive)
          {
            debug = 5;

            isShutdownTimerActive = true;
            shutdownTimerSeconds = SETTINGS_TURN_OFF_TIMEOUT;
          }
          else
          {
            debug = 6;

            if (shutdownTimerSeconds > 0)
            {
              debug = 7;

              shutdownTimerSeconds--;
            }
            else
            {
              debug = 8;
              shutdownPi();
              return;
            }
          }
        }
      }
    }
    else
    {
      if (isShutdownPending)
      {
        DepowerPi();
        return;
      }
    }
  }
}

void turnOnPi()
{
  if (isStartupPending)
  {
    return;
  }

  isStartupPending = true;
  isShutdownPending = false;
  isShutdownTimerActive = false;

  Serial.println("STARTUP");

  digitalWrite(RASPI_POWER_PIN, LOW);
  delay(100);
  
  digitalWrite(RASPI_SHUTDOWN_PIN, LOW);
  delay(100);
  
  digitalWrite(RASPI_SHUTDOWN_PIN, HIGH);
  delay(100);

}

void DepowerPi()
{
  isShutdownPending = false;
  isShutdownTimerActive = false;
  isStartupPending = false;

  Serial.println("POWER OFF");
  digitalWrite(RASPI_POWER_PIN, HIGH);
}

void shutdownPi()
{
  if (isShutdownPending)
  {
    return;
  }

  Serial.println("SHUTDOWN");

  isShutdownPending = true;
  isShutdownTimerActive = false;

  digitalWrite(RASPI_SHUTDOWN_PIN, LOW);
  delay(100);
  
  digitalWrite(RASPI_SHUTDOWN_PIN, HIGH);
  delay(100);  
}

void diag()
{
  Serial.print("Pi State: ");

  if (piState() == 1)
  {
    Serial.print("ON");
  }
  else
  {
    Serial.print("OFF");
  }

  Serial.print(" VRaw: "); Serial.print(analogRead(RASPI_CAPACITOR_PROBE_PIN));
  Serial.print(" V: "); Serial.print(capVoltage());
  Serial.print(" Cap: "); Serial.print(capCharge());
  Serial.print(" Ext: "); Serial.print(hasExternalPower());
  Serial.print(" isStartup : "); Serial.print(isStartupPending);
  Serial.print(" isShutdown : "); Serial.println(isShutdownPending);
}

void receiveEvent(int howMany)
{
  if (Wire.available())
  {
    i2cRegister = Wire.read();
  }

  if (Wire.available()) // write
  {
    i2cValue = Wire.read();

    switch (i2cRegister)
    {
      case 0x05:
        SETTINGS_TURN_OFF_TIMEOUT = i2cValue;
        saveSettings();
        break;

      case 0x06:
        if (i2cValue <= 100)
        {
          SETTINGS_TURN_OFF_THRESHOLD = i2cValue;
          saveSettings();
        }
        break;

      case 0x07:
        if (i2cValue <= 100)
        {
          SETTINGS_TURN_ON_THRESHOLD = i2cValue;
          saveSettings();
        }
        break;

      case 0x50:
        SETTINGS_I2C_ADDRESS = i2cValue;
        saveSettings();
        break;

      case 0x61:
        digitalWrite(RASPI_POWER_PIN, i2cValue);
        break;
    }

    i2cValue = 0;
  }
}


void requestEvent()
{
  switch (i2cRegister)
  {
    case 0x01:
      i2cValue = 0x64;
      break;

    case 0x02:
      i2cValue = hasExternalPower() ? 1 : 0;
      break;

    case 0x03:
      i2cValue = capCharge();
      break;

    case 0x04:
      i2cValue = capVoltage();
      break;

    case 0x05:
      i2cValue = SETTINGS_TURN_OFF_TIMEOUT;
      break;

    case 0x06:
      i2cValue = SETTINGS_TURN_OFF_THRESHOLD;
      break;

    case 0x07:
      i2cValue = SETTINGS_TURN_ON_THRESHOLD;
      break;

    case 0x50:
      i2cValue = SETTINGS_I2C_ADDRESS;
      break;



    case 0x71:
      i2cValue = piState();
      break;

    case 0x72:
      i2cValue = 2;
      break;

    case 0x73:
      i2cValue = debug;
      break;
  }

  Wire.write(i2cValue);
}

bool piState()
{
  return digitalRead(RASPI_STATE_PIN);
}

bool hasExternalPower()
{
  return digitalRead(RASPI_EXTERNAL_POWER_PROBE_PIN);
}

byte capCharge()
{
  int value = analogRead(RASPI_CAPACITOR_PROBE_PIN);

  if (value < 500)
  {
    return 0;
  }
  
  return lowByte(map(value, 500, 1023, 1, 100));
}


byte capVoltage()
{
  int value = analogRead(RASPI_CAPACITOR_PROBE_PIN);

  return lowByte(map(value, 0, 1023, 1, 50));
}


void loadSettings()
{
  for (byte i = 0; i < 10; i++)
  {
    settings[i] = EEPROM.read(i);
  }

  if (SETTINGS_IS_INITIALIZED != 2) // not initialized
  {
    SETTINGS_IS_INITIALIZED = 2;
    SETTINGS_I2C_ADDRESS = 0x19;
    SETTINGS_TURN_OFF_TIMEOUT = 3;
    SETTINGS_TURN_OFF_THRESHOLD = 50;
    SETTINGS_TURN_ON_THRESHOLD = 70;

    saveSettings();
  }

}

void saveSettings()
{
  for (byte i = 0; i < 10; i++)
  {
    EEPROM.write(i, settings[i]);
  }
}
