#include <SoftwareSerial.h>
#include <Servo.h>

#define RxD 7
#define TxD 6
#define ConnStatus A1

#define DEBUG_ENABLED  1

int shieldPairNumber = 5;

// CAUTION: If ConnStatusSupported = true you MUST NOT use pin A1 otherwise "random" reboots will occur
// CAUTION: If ConnStatusSupported = true you MUST set the PIO[1] switch to A1 (not NC)

// Set to "true" when digital connection status is available on Arduino pin
boolean ConnStatusSupported = true;   

// This is concatenated with shieldPairNumber later
String slaveNameCmd = "\r\n+STNA=Slave";   

SoftwareSerial blueToothSerial(RxD, TxD);

Servo servoL;
Servo servoR;

void setup()
{
  Serial.begin(9600);
  blueToothSerial.begin(38400);                    // Set Bluetooth module to default baud rate 38400

  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  pinMode(ConnStatus, INPUT);

//  servoL.attach(13);               // Attach left signal to pin 13
//  servoR.attach(12);

  if (ConnStatusSupported)
  {
    Serial.println("Checking Slave-Master connection status.");
  }

  if (ConnStatusSupported && digitalRead(ConnStatus) == 1)
  {
    Serial.println("Already connected to Master - remove USB cable if reboot of Master Bluetooth required.");
  }
  else
  {
    Serial.println("Not connected to Master.");
    // Set up the local (slave) Bluetooth module
    setupBlueToothConnection();
    // Wait one second
    delay(1000);
    // flush the serial buffers
    Serial.flush();
    blueToothSerial.flush();
  }
}

String command = "";
String manualCommand = "";
bool manualServoStopped = true;
double manualOriginX = 523;
double manualOriginY = 518;

int MODE_MANUAL = 1;
int MODE_AUTO = 2;
int MODE_MEM_FWD = 4;
int MODE_MEM_BAK = 8;

int currentMode = MODE_MANUAL;

void loop()
{
  while (blueToothSerial.available())
  {
    char recvChar = blueToothSerial.read();
    if (recvChar == '(')
    {
      command = "(";
    }
    else if (command.length() > 0 && recvChar == ')')
    {
      command += ')';
//      
//      if (command.equals("(MANUAL)"))
//      {
//        currentMode = MODE_MANUAL;
//      }
//      else if (command.equals("(AUTO,,)"))
//      {
//        currentMode = MODE_AUTO;
//      }
//      else if (command.equals("(MEM_FWD,,)"))
//      {
//        currentMode = MODE_MEM_FWD;
//      }
//      else if (command.equals("(MEM_BAK,,)"))
//      {
//        currentMode = MODE_MEM_BAK;
//      }
//      else if (command.startsWith("(ADJ_XY,"))
//      {
//        manualCommand = command;
//      }
//      else
//      {
//        manualCommand = command;
//      }

manualLoop(command);
      command = "";
    }
    else if (command.length() > 0)
    {
      command += recvChar;
    }
  }
  if (currentMode == MODE_MANUAL)
  {
    manualLoop(manualCommand);
  }
  if (currentMode == MODE_AUTO)
  {
    autolLoop();
  }
}

void autolLoop()
{
}

void manualLoop(String manualCommand)
{
  if (manualCommand.endsWith(")"))
  {
    if (manualCommand.startsWith("(ADJ_XY,"))
    {
      manualAdjustCenterCoordinate(manualCommand);
    }
    else
    {
      manualVectorToServoSpeed(manualCommand);
    }
  }
}

void manualVectorToServoSpeed(String manualCommand)
{
  String xVal = manualCommand.substring(1, manualCommand.indexOf(','));
  xVal.trim();

  manualCommand = manualCommand.substring(manualCommand.indexOf(',') + 1);
  String yVal = manualCommand.substring(0, manualCommand.indexOf(','));
  yVal.trim();

  manualCommand = manualCommand.substring(manualCommand.indexOf(',') + 1);
  String zVal = manualCommand.substring(0, manualCommand.indexOf(')'));
  zVal.trim();

  double rawX = xVal.toDouble();
  double rawY = yVal.toDouble();
  int rawZ = zVal.toInt();

  double shiftedX = rawX - manualOriginX;
  double shiftedY = rawY - manualOriginY;
  double magnitude = sqrt(shiftedX * shiftedX + shiftedY * shiftedY);

  if (magnitude <= 10)
  {
    manualStopServo();
  }
  else if (shiftedX >= 0 && shiftedY >= 0)
  {
    manualQ1VectorToServoSpeed(shiftedX, shiftedY);
  }
  else if (shiftedX <= 0 && shiftedY >= 0)
  {
    manualQ2VectorToServoSpeed(shiftedX, shiftedY);
  }
  else if (shiftedX >= 0 && shiftedY <= 0)
  {
    manualQ4VectorToServoSpeed(shiftedX, shiftedY);
  }
  else
  {
    manualQ3VectorToServoSpeed(shiftedX, shiftedY);
  }
}

void manualAdjustCenterCoordinate(String manualCommand)
{
  String prefix = manualCommand.substring(1, manualCommand.indexOf(','));
  prefix.trim();

  manualCommand = manualCommand.substring(manualCommand.indexOf(',') + 1);
  String xVal = manualCommand.substring(0, manualCommand.indexOf(','));
  xVal.trim();

  manualCommand = manualCommand.substring(manualCommand.indexOf(',') + 1);
  String yVal = manualCommand.substring(0, manualCommand.indexOf(')'));
  yVal.trim();

  manualOriginX = xVal.toInt();
  manualOriginY = yVal.toInt();
}

void manualQ1VectorToServoSpeed(double x, double y)
{
  manualServoStopped = false;
  servoL.attach(13);
  servoR.attach(12);
  double L = 1700.0;
  double R = 1700.0 - atan2(y, x) * 2.0 / PI * 400.0;
  double deltaL = L - 1500;
  double deltaR = R - 1500;
  double radius = 512.0;
  double mananitude = sqrt(pow(x, 2) + pow(y, 2));
  int newL = round(L - deltaL * (1.0 - (mananitude / radius)));
  int newR = round(R - deltaR * (1.0 - (mananitude / radius)));
  servoL.writeMicroseconds(manualRefitSpeed(newL));
  servoR.writeMicroseconds(manualRefitSpeed(newR));
}

void manualQ2VectorToServoSpeed(double x, double y)
{
  manualServoStopped = false;
  servoL.attach(13);
  servoR.attach(12);
  double L = 1700.0 - atan2(y, x) / PI * 400.0;
  double R = 1300;
  double deltaL = L - 1500;
  double deltaR = R - 1500;
  double radius = 512.0;
  double mananitude = sqrt(pow(x, 2) + pow(y, 2));
  int newL = round(L - deltaL * (1.0 - (mananitude / radius)));
  int newR = round(R - deltaR * (1.0 - (mananitude / radius)));
  servoL.writeMicroseconds(manualRefitSpeed(newL));
  servoR.writeMicroseconds(manualRefitSpeed(newR));
}

void manualQ3VectorToServoSpeed(double x, double y)
{
  manualServoStopped = false;
  servoL.attach(13);
  servoR.attach(12);
  double L = 1300.0;
  double R = 1300.0 + atan2(abs(y), abs(x)) * 2.0 / PI * 400.0;
  double deltaL = L - 1500;
  double deltaR = R - 1500;
  double radius = 512.0;
  double mananitude = sqrt(pow(x, 2) + pow(y, 2));
  int newL = round(L - deltaL * (1.0 - (mananitude / radius)));
  int newR = round(R - deltaR * (1.0 - (mananitude / radius)));
  servoL.writeMicroseconds(manualRefitSpeed(newL));
  servoR.writeMicroseconds(manualRefitSpeed(newR));
}

void manualQ4VectorToServoSpeed(double x, double y)
{
  manualServoStopped = false;
  servoL.attach(13);
  servoR.attach(12);
  double L = 1700.0 - atan2(abs(y), abs(x)) * 2.0 / PI * 400.0;
  double R = 1700.0;
  double deltaL = L - 1500;
  double deltaR = R - 1500;
  double radius = 512.0;
  double mananitude = sqrt(pow(x, 2) + pow(y, 2));
  int newL = round(L - deltaL * (1.0 - (mananitude / radius)));
  int newR = round(R - deltaR * (1.0 - (mananitude / radius)));
  servoL.writeMicroseconds(manualRefitSpeed(newL));
  servoR.writeMicroseconds(manualRefitSpeed(newR));
}

int manualRefitSpeed(int v)
{
  if (v > 1700)
  {
    return 1700;
  }
  if (v < 1300)
  {
    return 1300;
  }
  return v;
}

void manualStopServo()
{
  if (manualServoStopped)
  {
    return;
  }

  servoL.writeMicroseconds(1500);
  servoR.writeMicroseconds(1500);
  servoL.detach();
  servoR.detach();
  manualServoStopped = true;
}

void setupBlueToothConnection()
{
  Serial.println("Setting up the local (slave) Bluetooth module.");

  slaveNameCmd += shieldPairNumber;
  slaveNameCmd += "\r\n";

  blueToothSerial.print("\r\n+STWMOD=0\r\n");      // Set the Bluetooth to work in slave mode
  blueToothSerial.print(slaveNameCmd);             // Set the Bluetooth name using slaveNameCmd
  blueToothSerial.print("\r\n+STAUTO=0\r\n");      // Auto-connection should be forbidden here
  blueToothSerial.print("\r\n+STOAUT=1\r\n");      // Permit paired device to connect me

  //  print() sets up a transmit/outgoing buffer for the string which is then transmitted via interrupts one character at a time.
  //  This allows the program to keep running, with the transmitting happening in the background.
  //  Serial.flush() does not empty this buffer, instead it pauses the program until all Serial.print()ing is done.
  //  This is useful if there is critical timing mixed in with Serial.print()s.
  //  To clear an "incoming" serial buffer, use while(Serial.available()){Serial.read();}

  blueToothSerial.flush();
  delay(2000);                                     // This delay is required

  blueToothSerial.print("\r\n+INQ=1\r\n");         // Make the slave Bluetooth inquirable

  blueToothSerial.flush();
  delay(2000);                                     // This delay is required

  Serial.println("The slave bluetooth is inquirable!");
}
