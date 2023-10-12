#include <Servo.h>                      // Include servo library
/*
 * Robotics with the BOE Shield - TestLeftIR
 * Display 1 if the left IR detector does not detect an object,
 * or 0 if it does.
 */
Servo servoLeft;                        // Declare left and right servos
Servo servoRight;
double distance_front;
void setup()                                 // Built-in initialization block
{
  tone(4, 3000, 1000);                       // Play tone for 1 second
  delay(1000);                               // Delay to finish tone

  servoLeft.attach(13);               // Attach left signal to pin 13
  servoRight.attach(12);

  pinMode(10, INPUT);  pinMode(9, OUTPUT);   // Left IR LED & Receiver
  pinMode(3, INPUT);  pinMode(2, OUTPUT);   // Right IR LED & Receiver

  Serial.begin(9600);                        // Set data rate to 9600 bps

}  


void loop()                                  // Main loop auto-repeats
{

  distance_front = getDistanceInCM();
  Serial.print(distance_front);Serial.println("; ");
  int irLeft = irDetect(9, 10, 38000);       // left heck for object

  int irRight = irDetect(2, 3, 38000);       // right heck for object

  Serial.print("left value: ");                    // Display 1/0 no detect/detect
  Serial.println(irLeft);
  Serial.print("right value: ");                    // Display 1/0 no detect/detect
  Serial.println(irRight);

  if (distance_front <= 7) {
//    if (irLeft == 0 && irRight == 0) {
//      no_move();
//    }
    if (irRight == 1 && irLeft == 0) {
      move_backward();
      delay(200);
      turn_right();
      delay(400);
    } else if (irRight == 0 && irLeft == 1) {
      move_backward();
      delay(200);
      turn_left();
      delay(400);
    } else {
      move_backward();
      delay(200);
    }
//    if (irLeft == 0 ) {
//      turn_right();
//      delay(200);
//      no_move();
//      delay(100);
//    } else {
//      turn_left();
//      delay(200);
//      no_move();
//      delay(100);
//    }
  }
  
  else if (distance_front < 9 &&distance_front > 7 )
  {
    no_move();
    delay(500);
    if (irRight == 1 && irLeft == 0 ){
      turn_right();
      delay(600);
      no_move();
      delay(100);
    } else {
      turn_left();
      delay(200);
      no_move();
      delay(100);
    }
      
  } 

  move_forward();
 

  delay(10);                                // 0.1 second delay
}

// IR Object Detection Function

int irDetect(int irLedPin, int irReceiverPin, long frequency)
{
  tone(irLedPin, frequency, 8);              // IRLED 38 kHz for at least 1 ms
  delay(1);                                  // Wait 1 ms
  int ir = digitalRead(irReceiverPin);       // IR receiver -> ir variable
  delay(1);                                  // Down time before recheck
  return ir;                                 // Return 1 no detect, 0 detect
}  

void no_move()
{
 servoLeft.writeMicroseconds(1500);
 servoRight.writeMicroseconds(1500); 
}

void move_forward()
 {
   servoLeft.writeMicroseconds(1700);
   servoRight.writeMicroseconds(1300); 
 }

  void move_backward()
 {
   servoLeft.writeMicroseconds(1300);
   servoRight.writeMicroseconds(1700); 
 }

   void turn_right()
 {
   servoLeft.writeMicroseconds(1700);
   servoRight.writeMicroseconds(1700); 
 }

    void turn_left()
 {
   servoLeft.writeMicroseconds(1300);
   servoRight.writeMicroseconds(1300); 
 }

 double getDistanceInCM()
 {
  int echoPin = 6;
  int trigPin = 5;
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  double duration = pulseIn(echoPin, HIGH);
  double distance = duration * 0.034 / 2;
  return distance;
 
  //Serial.print("Distance: ");
  //Serial.print(distance);
  //Serial.println(" cm");
}

