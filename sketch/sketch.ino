#include "CurieIMU.h"
#include <MadgwickAHRS.h>
#include <CurieBLE.h>
#include <Adafruit_NeoPixel.h>
#define LED1 3
#define LED2 5
#define LED3 6
#define LED4 9

Adafruit_NeoPixel pixel1 = Adafruit_NeoPixel(1, LED1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixel2 = Adafruit_NeoPixel(1, LED2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixel3 = Adafruit_NeoPixel(1, LED3, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixel4 = Adafruit_NeoPixel(1, LED4, NEO_GRB + NEO_KHZ800);

unsigned long previousMillis = 0;

const long interval = 500;

int ledState = LOW;

int side, oldSide;

char mode = '0';
// 0 = standby
// 1 = rotation
// 2 = swipe
// 3 = potentiometer
// 4 = face

char control = '0';

int rotation = 0;
// 1 right
// 2 left
// 0 wait

int swipe = 0;
// 1 swipe
// 0 wait

float matrix[6][3] = {
  { -1, 0, 0},
  {0, -1, 0},
  {0, 0, -1},
  {1, 0, 0},
  {0, 1, 0},
  {0, 0, 1},
};

int oldDistance, distance;

int lastOrientation = - 1, orientation; // previous orientation (for comparison)

Madgwick filter;
unsigned long microsPerReading, microsPrevious;
float accelScale, gyroScale;
float defaultAccRate;
const int ledPin =  LED_BUILTIN;
float offset;
bool check = false;

BLEPeripheral blePeripheral;

//------------------------------------------------------------------------ BLE SERVICES
BLEService modeService("988dbab9-a657-45fe-80ef-9f9bed761947");
BLEService rotationService("988dbab9-a657-45fe-80ef-9f9bed761948");
BLEService ledService("988dbab9-a657-45fe-80ef-9f9bed761949");
BLEService swipeService("988dbab9-a657-45fe-80ef-9f9bed761950");
BLEService potService("988dbab9-a657-45fe-80ef-9f9bed761951");


//------------------------------------------------------------------------ BLE CHARACTERISTICS
BLEUnsignedCharCharacteristic rotationChar("06abf104-c81d-4d93-bce5-bc6b38188ba5", BLERead | BLEWrite | BLENotify);
BLECharCharacteristic switchChar("06abf104-c81d-4d93-bce5-bc6b38188ba4", BLERead | BLEWrite);
BLECharacteristic ledChar("06abf104-c81d-4d93-bce5-bc6b38188ba3", BLERead | BLEWrite, 16);
BLEUnsignedCharCharacteristic swipeChar("06abf104-c81d-4d93-bce5-bc6b38188ba2", BLERead | BLEWrite | BLENotify);
BLECharCharacteristic potChar("06abf104-c81d-4d93-bce5-bc6b38188ba1", BLERead | BLENotify);


//------------------------------------------------------------------------ TEST

void changeColor(const char control[16]) {
  char _red[4], _green[4], _blue[4], _brightness[4], _white[4];
  int red, green, blue, brightness, white, offset;
  _red[3] = '\0';
  _green[3] = '\0';
  _blue[3] = '\0';
  _white[3] = '\0';
  _brightness[3] = '\0';
  offset = 1;
  for (int i = 0; i < 3; i++) {
    _red[i] = control[i + offset];
  }
  offset = 4;
  for (int i = 0; i < 3; i++) {
    _green[i] = control[i + offset];
  }
  offset = 7;
  for (int i = 0; i < 3; i++) {
    _blue[i] = control[i + offset];
  }
  offset = 10;
  for (int i = 0; i < 3; i++) {
    _white[i] = control[i + offset];
  }
  offset = 13;
  for (int i = 0; i < 3; i++) {
    _brightness[i] = control[i + offset];
  }
  red = atoi(_red);
  green = atoi(_green);
  blue = atoi(_blue);
  white = atoi(_white);
  brightness = atoi(_brightness);

  if (control[0] == '3') {
    pixel1.setPixelColor(0, red, green, blue, white);
    pixel1.setBrightness(brightness);
    pixel1.show();
  }
  if (control[0] == '5') {
    pixel2.setPixelColor(0, red, green, blue, white);
    pixel2.setBrightness(brightness);
    pixel2.show();
  }
  if (control[0] == '6') {
    pixel3.setPixelColor(0, red, green, blue, white);
    pixel3.setBrightness(brightness);
    pixel3.show();
  }
  if (control[0] == '9') {
    pixel4.setPixelColor(0, red, green, blue, white);
    pixel4.setBrightness(brightness);
    pixel4.show();
  }
}

void rxCharacteristicWritten(BLECentral& central, BLECharacteristic& characteristic) {
  if (characteristic.value()) {       //check if there is value
    char control[16];
    strncpy(control, (char *)characteristic.value(), 16);
    Serial.println(control);
    changeColor(control);
  }

}




//------------------------------------------------------------------------ BLE EVENT HANDLERS
void switchMode(BLECentral& central, BLECharacteristic& characteristic) {
  // central wrote new value to characteristic
  Serial.print("Characteristic event, written: ");
  if (switchChar.value() == 1) {
    Serial.println("Rotation mode");
  } else if (switchChar.value() == 2) {
    Serial.println("Swipe mode");
  }
  else if (switchChar.value() == 3) {
    Serial.println("Potentiometer mode");
  }
}

void blePeripheralConnectHandler(BLECentral& central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
}


//------------------------------------------------------------------------ BLE SETUP
void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);
  changeColor("3000000000000020");
  changeColor("5000000000000020");
  changeColor("6000000000000020");
  changeColor("9000000000000020");
  Serial.begin(9600);
  // initialize device
  Serial.println("Initializing IMU device...");
  CurieIMU.begin();
  defaultAccRate = CurieIMU.getAccelerometerRate();

  /*
    CurieIMU.autoCalibrateAccelerometerOffset(X_AXIS, 0);
    CurieIMU.autoCalibrateAccelerometerOffset(Y_AXIS, 0);
    CurieIMU.autoCalibrateAccelerometerOffset(Z_AXIS, 1);
  */

  // Set the accelerometer range to 2G
  CurieIMU.setAccelerometerRange(2);
  CurieIMU.setGyroRate(25);
  filter.begin(25);

  // Set the gyroscope range to 250 degrees/second
  CurieIMU.setGyroRange(250);
  microsPerReading = 1000000 / 25;
  microsPrevious = micros();

  // BLE
  // set the local name peripheral advertises
  blePeripheral.setLocalName("CUBE");
  // set the UUID for the service this peripheral advertises
  blePeripheral.setAdvertisedServiceUuid(modeService.uuid());

  // add services and characteristics
  blePeripheral.addAttribute(modeService);
  blePeripheral.addAttribute(switchChar);
  blePeripheral.addAttribute(rotationService);
  blePeripheral.addAttribute(rotationChar);
  blePeripheral.addAttribute(ledService);
  blePeripheral.addAttribute(ledChar);
  blePeripheral.addAttribute(swipeService);
  blePeripheral.addAttribute(swipeChar);
  blePeripheral.addAttribute(potService);
  blePeripheral.addAttribute(potChar);

  // assign event handlers for connected, disconnected to peripheral
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // assign event handlers for characteristics
  switchChar.setEventHandler(BLEWritten, switchMode);
  ledChar.setEventHandler(BLEWritten, rxCharacteristicWritten);
  // set an initial value for the characteristics
  switchChar.setValue(0);
  rotationChar.setValue(rotation);
  swipeChar.setValue(swipe);

  // advertise the services
  blePeripheral.begin();
  Serial.println(("Bluetooth device active, waiting for connections..."));

  //lights up LED on power on
  changeColor("3000255000000050");
  changeColor("5000255000000050");
  changeColor("6000255000000050");
  changeColor("9000255000000050");
  delay(1000);
  changeColor("3000000000000000");
  changeColor("5000000000000000");
  changeColor("6000000000000000");
  changeColor("9000000000000000");



}



void loop() {
  unsigned long currentMillis = millis();

  if (switchChar.value() == 0) { // STAND BY

  }

  if (switchChar.value() == 1) { // ROTATION MODE
    orientation = - 1;   // the board's orientation
    /*
      The orientations of the board:
      1: flat, processor facing up
      2: flat, processor facing down
      3: landscape, analog pins down
      4: landscape, analog pins up
      5: portrait, USB connector up
      6: portrait, USB connector down
    */
    // read accelerometer:
    int x = CurieIMU.readAccelerometer(X_AXIS);
    int y = CurieIMU.readAccelerometer(Y_AXIS);
    int z = CurieIMU.readAccelerometer(Z_AXIS);

    // calculate the absolute values, to determine the largest
    int absX = abs(x);
    int absY = abs(y);
    int absZ = abs(z);

    if ( (absZ > absX) && (absZ > absY)) {
      // base orientation on Z
      if (z > 0) {
        orientation = 1;
      } else {
        orientation = 2;
      }
    } else if ( (absY > absX) && (absY > absZ)) {
      // base orientation on Y
      if (y > 0) {
        orientation = 3;
      } else {
        orientation = 4;
      }
    } else {
      // base orientation on X
      if (x < 0) {
        orientation = 5;
      } else {
        orientation = 6;
      }
    }

    //Serial.println(orientation);

    // current previous rotation
    //     1       3        dx
    //     1       4        sx
    //     4       1        dx
    //     4       2        sx
    //     3       2        dx
    //     3       1        sx
    //     2       4        dx
    //     2       3        sx

    // if the orientation has changed, check for rotations:
    if (orientation != lastOrientation) {
      // if for checkin rotation
      switch (orientation) {
        case 1:
          if (lastOrientation == 3) {
            rotationChar.setValue(2);
            Serial.println("DX Rotation");
            allLedOn(1);
          }
          else if (lastOrientation == 4) {
            rotationChar.setValue(1);
            Serial.println("SX Rotation");
            allLedOn(0);
          }
          break;
        case 2:
          if (lastOrientation == 4) {
            rotationChar.setValue(2);
            Serial.println("DX Rotation");
            allLedOn(1);
          }
          else if (lastOrientation == 3) {
            rotationChar.setValue(1);
            Serial.println("SX Rotation");
            allLedOn(0);
          }
          break;
        case 3:
          if (lastOrientation == 2) {
            rotationChar.setValue(2);
            Serial.println("DX Rotation");
            allLedOn(1);
          }
          else if (lastOrientation == 1) {
            rotationChar.setValue(1);
            Serial.println("SX Rotation");
            allLedOn(0);
          }
          break;
        case 4:
          if (lastOrientation == 1) {
            rotationChar.setValue(2);
            Serial.println("DX Rotation");
            allLedOn(1);
          }
          else if (lastOrientation == 2) {
            rotationChar.setValue(1);
            Serial.println("SX Rotation");
            allLedOn(0);
          }
          break;
      }
      lastOrientation = orientation;
      delay(1000);
      allLedOff();
    }
  }


  /*
    if (switchChar.value() == 1) { // ROTATION MODE
      // read accelerometer:
      CurieIMU.setAccelerometerRate(defaultAccRate);
      float x = CurieIMU.readAccelerometer(X_AXIS);
      float y = CurieIMU.readAccelerometer(Y_AXIS);
      float z = CurieIMU.readAccelerometer(Z_AXIS);
      oldSide = cubeSide(x, y, z);
      // if the orientation has changed, print out a description:
      side = cubeSide(CurieIMU.readAccelerometer(X_AXIS), CurieIMU.readAccelerometer(Y_AXIS), CurieIMU.readAccelerometer(Z_AXIS));

      if (side != oldSide) {

        if (side == 5) {
          Serial.print(side);
          Serial.print(" ");
          if (oldSide == 4) {
            Serial.println("DX rotation");
            rotationChar.setValue(2);
            changeColor("3000255000000050");
            changeColor("5000255000000050");
            changeColor("6000255000000050");
            changeColor("9000255000000050");
          }
          if (oldSide == 1) {
            Serial.println("SX rotation");
            rotationChar.setValue(1);
            changeColor("3000000255000050");
            changeColor("3000000255000050");
            changeColor("6000000255000050");
            changeColor("9000000255000050");
          }
        }

        if (side == 1) {
          Serial.print(side);
          Serial.print(" ");
          if (oldSide == 5) {
            Serial.println("DX rotation");
            rotationChar.setValue(2);
            changeColor("3000255000000050");
            changeColor("5000255000000050");
            changeColor("6000255000000050");
            changeColor("9000255000000050");
          }
          if (oldSide == 2) {
            Serial.println("SX rotation");
            rotationChar.setValue(1);
            changeColor("3000000255000050");
            changeColor("5000000255000050");
            changeColor("6000000255000050");
            changeColor("9000000255000050");
          }
        }

        if (side == 4) {
          Serial.print(side);
          Serial.print(" ");
          if (oldSide == 2) {
            Serial.println("DX rotation");
            rotationChar.setValue(2);
            changeColor("3000255000000050");
            changeColor("5000255000000050");
            changeColor("6000255000000050");
            changeColor("9000255000000050");
          }
          if (oldSide == 5) {
            Serial.println("SX rotation");
            rotationChar.setValue(1);
            changeColor("3000000255000050");
            changeColor("3000000255000050");
            changeColor("6000000255000050");
            changeColor("9000000255000050");
          }
        }

        if (side == 2) {
          Serial.print(side);
          Serial.print(" ");
          if (oldSide == 1) {
            Serial.println("DX rotation");
            rotationChar.setValue(2);
            changeColor("3000255000000050");
            changeColor("5000255000000050");
            changeColor("6000255000000050");
            changeColor("9000255000000050");
          }
          if (oldSide == 4) {
            Serial.println("SX rotation");
            rotationChar.setValue(1);
            changeColor("3000000255000050");
            changeColor("3000000255000050");
            changeColor("6000000255000050");
            changeColor("9000000255000050");
          }
        }
        //Serial.println(side);
        oldSide = side;
        delay(1000);
        changeColor("3000000000000050");
        changeColor("5000000000000050");
        changeColor("6000000000000050");
        changeColor("9000000000000050");
      }

    }
  */

  if (switchChar.value() == 2) { // SWIPE MODE
    oldDistance = analogRead(A0);
    delay(500);
    distance = analogRead(A0);
    if (distance - oldDistance > 100) {
      Serial.println("SWIPE");
      swipeChar.setValue(1);
      //
      changeColor("3255000000000050");
      changeColor("5255000000000050");
      changeColor("6255000000000050");
      changeColor("9255000000000050");
      delay(1000);
    }
    changeColor("3000000000000050");
    changeColor("5000000000000050");
    changeColor("6000000000000050");
    changeColor("9000000000000050");
    oldDistance = distance;
  }

  if (switchChar.value() == 3) { // POTENTIOMETER MODE
    CurieIMU.setAccelerometerRate(25);
    int aix, aiy, aiz;
    int gix, giy, giz;
    float ax, ay, az;
    float gx, gy, gz;
    int brigth;
    //float roll, pitch;
    float heading;
    unsigned long microsNow;

    // check if it's time to read data and update the filter
    microsNow = micros();
    if (microsNow - microsPrevious >= microsPerReading) {

      // read raw data from CurieIMU
      CurieIMU.readMotionSensor(aix, aiy, aiz, gix, giy, giz);

      // convert from raw data to gravity and degrees/second units
      ax = convertRawAcceleration(aix);
      ay = convertRawAcceleration(aiy);
      az = convertRawAcceleration(aiz);
      gx = convertRawGyro(gix);
      gy = convertRawGyro(giy);
      gz = convertRawGyro(giz);

      // update the filter, which computes orientation
      filter.updateIMU(gx, gy, gz, ax, ay, az);

      // print the heading, pitch and roll

      if (check == false) {
        CurieIMU.autoCalibrateGyroOffset();
        CurieIMU.autoCalibrateAccelerometerOffset(X_AXIS, 0);
        CurieIMU.autoCalibrateAccelerometerOffset(Y_AXIS, 0);
        CurieIMU.autoCalibrateAccelerometerOffset(Z_AXIS, 1);
        changeColor("3100100000000020");
        changeColor("5100100000000020");
        changeColor("6100100000000020");
        changeColor("9100100000000020");
        for (int i = 0; i < 10; i++) {
          offset += filter.getYaw();
        }
        offset = offset / 10;
        check = true;
      }
      heading = filter.getYaw();
      heading = heading - offset;
      heading = round(heading);
      Serial.println(heading);
      potChar.setValue(heading);
      delay(500);
      brigth = 100 + (int)heading * 8;
      Serial.println(brigth);
      pixel1.setPixelColor(0, brigth, brigth, 0, 0);
      pixel2.setPixelColor(0, brigth, brigth, 0, 0);
      pixel3.setPixelColor(0, brigth, brigth, 0, 0);
      pixel4.setPixelColor(0, brigth, brigth, 0, 0);
      pixel1.show();
      pixel2.show();
      pixel3.show();
      pixel4.show();
      // increment previous time, so we keep proper pace
      microsPrevious = microsPrevious + microsPerReading;
    }
  }

  if (switchChar.value() == 4) { // FACE MODE
    int orientation;
    /*
      The orientations of the board:
      1: flat, processor facing up
      2: flat, processor facing down
      3: landscape, analog pins down
      4: landscape, analog pins up
      5: portrait, USB connector up
      6: portrait, USB connector down
    */
    // read accelerometer:
    int x = CurieIMU.readAccelerometer(X_AXIS);
    int y = CurieIMU.readAccelerometer(Y_AXIS);
    int z = CurieIMU.readAccelerometer(Z_AXIS);

    // calculate the absolute values, to determine the largest
    int absX = abs(x);
    int absY = abs(y);
    int absZ = abs(z);

    if ( (absZ > absX) && (absZ > absY)) {
      // base orientation on Z
      if (z > 0) {
        orientation = 1;
      } else {
        orientation = 2;
      }
    } else if ( (absY > absX) && (absY > absZ)) {
      // base orientation on Y
      if (y > 0) {
        orientation = 3;
      } else {
        orientation = 4;
      }
    } else {
      // base orientation on X
      if (x < 0) {
        orientation = 5;
      } else {
        orientation = 6;
      }
    }
    Serial.println(orientation);
    pixel1.setPixelColor(0, 255, 0, 0, 0);
    pixel2.setPixelColor(0, 255, 0, 0, 0);
    pixel3.setPixelColor(0, 255, 0, 0, 0);
    pixel4.setPixelColor(0, 255, 0, 0, 0);
    pixel1.show();
    pixel2.show();
    pixel3.show();
    pixel4.show();
    delay(500 * orientation);
    pixel1.setPixelColor(0, 0, 0, 0, 0);
    pixel2.setPixelColor(0, 0, 0, 0, 0);
    pixel3.setPixelColor(0, 0, 0, 0, 0);
    pixel4.setPixelColor(0, 0, 0, 0, 0);
    pixel1.show();
    pixel2.show();
    pixel3.show();
    pixel4.show();
    delay(500 * orientation);
  }

  //DEBUG
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }
}

int cubeSide(float Ax, float Ay, float Az) {
  float largest_dot = 0;
  int closest_side = -1; // will return -1 in case of a zero A vector
  for (int side = 0; side < 6; side++) {
    float dot = (matrix[side][0] * Ax) + (matrix[side][1] * Ay) + (matrix[side][2] * Az);
    if (dot > largest_dot) {
      largest_dot = dot;
      closest_side = side;
    }
  }
  return closest_side;
}

float convertRawAcceleration(int aRaw) {
  // since we are using 2G range
  // -2g maps to a raw value of -32768
  // +2g maps to a raw value of 32767

  float a = (aRaw * 2.0) / 32768.0;
  return a;
}

float convertRawGyro(int gRaw) {
  // since we are using 250 degrees/seconds range
  // -250 maps to a raw value of -32768
  // +250 maps to a raw value of 32767

  float g = (gRaw * 250.0) / 32768.0;
  return g;
}

void allLedOn(int i) {
  if (i == 1) {
    changeColor("3255000000000255");
    changeColor("5255000000000255");
    changeColor("6255000000000255");
    changeColor("9255000000000255");
  }
  else if (i==2) {
    changeColor("3000255000000255");
    changeColor("5000255000000255");
    changeColor("6000255000000255");
    changeColor("9000255000000255");
  }
  else if (i==3){
    changeColor("3000000255000255");
    changeColor("5000000255000255");
    changeColor("6000000255000255");
    changeColor("9000000255000255");
  }
  
}

void allLedOff() {
  changeColor("3000000000000255");
  changeColor("5000000000000255");
  changeColor("6000000000000255");
  changeColor("9000000000000255");

}



