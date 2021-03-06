const int TRIG = 13; // Assigne la valeur du pin TRIGGER
const int ECHO = 11; // Assigne la valeur du pin ECHO

const int TCRT_AVANT = A1; // Assigne la valeur du pin assigner au détecteur avant
const int TCRT_ARRIERE = A0; // Assigne la valeur du pin assigne au détecteur arrière

const int MOTEUR_G_PWM = 9;  // Assigne la valeur du pin assigner au PWM des moteurs gauches
const int MOTEUR_G_DIR_1 = 7; // Assigne la valeur du pin assigner à la direction 1 des moteurs gauches
const int MOTEUR_G_DIR_2 = 12; // Assigne la valeur du pin assigner à la direction 2 des moteurs gauches
const int MOTEUR_D_PWM = 10;  // Assigne la valeur du pin assigner au PWM des moteurs droits
const int MOTEUR_D_DIR_1 = A2; // Assigne la valeur du pin assigner à la direction 1 des moteurs droits
const int MOTEUR_D_DIR_2 = A3; // Assigne la valeur du pin assigner à la direction 2 des moteurs droits

const int MAX_DISTANCE = 77; // The maximum distance that the enemy can be
const int ATTACK_DISTANCE = 20; // The distance where the sumobot start attacking
const int MIN_DISTANCE = 2; // The minimum distance with the enemy to engage evasive manoeuvers

const int PUSH = 8;

const int FULL_SPEED = 100;
const int MAX_SPEED = 50;
const int SPEED = 40;
const int MIN_SPEED = 30;

const int WHITE_LINE_MIN_VALUE = 700;

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("ETALP - STARTING...");
  
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(TCRT_AVANT, INPUT); // Initialise le pin TCRT_AVANT comme entrée
  pinMode(TCRT_ARRIERE, INPUT); // Initialise le pin TCRT_ARRIERE comme entrée

  pinMode(PUSH, INPUT_PULLUP);

  pinMode(MOTEUR_G_PWM, OUTPUT); // Initalise le pin MOTEUR_G_PWM comme sortie
  pinMode(MOTEUR_G_DIR_1, OUTPUT); // Idem avec MOTEUR_G_DIR_1
  pinMode(MOTEUR_G_DIR_2, OUTPUT); // Idem avec MOTEUR_G_DIR_2
  pinMode(MOTEUR_D_PWM, OUTPUT); // Idem avec MOTEUR_D_PWM
  pinMode(MOTEUR_D_DIR_1, OUTPUT); // Idem avec MOTEUR_D_DIR_1
  pinMode(MOTEUR_D_DIR_2, OUTPUT); // Idem avec MOTEUR_D_DIR_2

  // Interruption doesn't detect any change
  /*interrupts();
  attachInterrupt(TCRT_AVANT, forwardLine, RISING);
  attachInterrupt(TCRT_ARRIERE, backwardLine, RISING);*/

  Serial.println("ETALP - Started!");

  while(digitalRead(PUSH)){
    delay(1);
  }

  delay(5000);
  Serial.println("ETALP - LAUNCHING...");
}

void loop() {
  while(true){
    int whiteLines = checkWhiteLine();

    if(whiteLines != 0){
      engageEvasiveManeuver(whiteLines);
      continue;
    }
    
    bool enemyDetected = findEnemy();

    if(enemyDetected){
      attack();
    } else {
      delay(500);
    }
  }
}


int checkWhiteLine(){
  Serial.println(analogRead(TCRT_AVANT));
  Serial.println(analogRead(TCRT_ARRIERE));
  if(analogRead(TCRT_AVANT) >= WHITE_LINE_MIN_VALUE && analogRead(TCRT_ARRIERE) >= WHITE_LINE_MIN_VALUE){
    return 2;
  }else if(analogRead(TCRT_AVANT) >= WHITE_LINE_MIN_VALUE){
    return 1;
  }else if(analogRead(TCRT_ARRIERE) >= WHITE_LINE_MIN_VALUE){
    return -1;
  }
  
  return 0;
}

void forwardLine(){
  engageEvasiveManeuver(1);
}

void backwardLine(){
  engageEvasiveManeuver(-1);
}

void engageEvasiveManeuver(int maneuver){
  stopEngine();
  
  if(maneuver == -1){ // Back sensor has detected white line
    rightEngine(FULL_SPEED, 1, 0);
    leftEngine(FULL_SPEED, 1, 0);
    Serial.println("Back sensors have detected somethings!");
    delay(1000);
    stopEngine();
  } else if(maneuver == 1){ // Forward sensors have detected white line
    rightEngine(FULL_SPEED, 0, 1);
    leftEngine(FULL_SPEED, 0, 1);
    Serial.println("Forward sensors have detected somethings!");
    delay(1000);
    stopEngine();
  } else if(maneuver == 2) {
    startRotationRight(SPEED);
    Serial.println("BOTH sensors have detected somethings!");
    delay(500);
    stopEngine();
  }
}



boolean findEnemy() {
  float distance = detect();
  unsigned long detectionTime = -1;
  unsigned long lastDetectionTime = -1;
  int timeElapsed = -1;

  startRotationLeft(MIN_SPEED);
  
  while(true){
    int maneuver = checkWhiteLine();
    distance = detect();
    
    if(!maneuver && distance > MIN_DISTANCE){
      engageEvasiveManeuver(maneuver);
    } else if(maneuver != 0){
      engageEvasiveManeuver(maneuver);
      startRotationLeft(MIN_SPEED);
      detectionTime = -1;
      lastDetectionTime = -1;
      timeElapsed = -1;
    }

    if(timeElapsed == -1){
      if((distance > MAX_DISTANCE || distance <= 0) && lastDetectionTime > 0){
        timeElapsed = (int)(lastDetectionTime - detectionTime);
        timeElapsed = (timeElapsed < 0) ? -timeElapsed : timeElapsed;
      } else if(distance > 0) {
        if(detectionTime == -1){
          detectionTime = millis();
          Serial.println("Enemy detected! (Start)");
        }
      } else{
        timeElapsed = (int)(lastDetectionTime - detectionTime);
        Serial.print("Enemy detected! Centering...");
      }
    } else{
      stopEngine();
      delay(1000);
      centerPosition(timeElapsed);
      return true;
    }

    delay(50);
  }

  return false;
}

float detect() {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(20);
  digitalWrite(TRIG, LOW);

  int timeToTarget = pulseIn(ECHO, HIGH);
  float distance = (float)(timeToTarget * 17) / 1000.0;
    
  Serial.print("Object detected at ");
  Serial.print(distance);
  Serial.println(" cm.");

  return distance;
}

void centerPosition(int timeElapsed){
  Serial.println("ETALP - Centering...");
  startRotationRight(MIN_SPEED);
  Serial.println(timeElapsed);
  delay((int)(timeElapsed / 2));
  stopEngine();
  Serial.println("ETALP - Centered!");
}



void attack(){
  Serial.println("ETALP - Attacking ennemy...");
  forward(SPEED);

  float distance = detect();

  while(distance > 0 && distance <= MAX_DISTANCE){
    delay(200);
    distance = detect();

    int maneuver = checkWhiteLine();

    if(maneuver != 0){
      engageEvasiveManeuver(maneuver);
    }
  }

  backward(FULL_SPEED);
  delay(200);
  stopEngine();
}



void forward(int speed){
  rightEngine(speed, 1, 0);
  leftEngine(speed, 1, 0);
}

void backward(int speed){
  rightEngine(speed, 0, 1);
  leftEngine(speed, 0, 1);
}

void startRotationLeft(int speed){
  leftEngine(speed, 0, 1);
  rightEngine(speed, 1, 0);
}

void startRotationRight(int speed){
  rightEngine(speed, 0, 1);
  leftEngine(speed, 1, 0);
}

void rightEngine(int speed, int direction, int opposite){
  digitalWrite(MOTEUR_D_DIR_1, direction); // On met la direction avant des moteurs droits à direction
  digitalWrite(MOTEUR_D_DIR_2, opposite); // On met la direction arrière des moteurs droits à opposite (l'opposée de la direction avant)
  analogWrite(MOTEUR_D_PWM, speed); // On applique la vitesse choisis par l'utilisateur (en quart)
}

void leftEngine(int speed, int direction, int opposite){
  digitalWrite(MOTEUR_G_DIR_1, direction); // On met la direction avant des moteurs droits à direction
  digitalWrite(MOTEUR_G_DIR_2, opposite); // On met la direction arrière des moteurs droits à opposite (l'opposée de la direction avant)
  analogWrite(MOTEUR_G_PWM, speed); // On applique la vitesse choisis par l'utilisateur (en quart)
}

void stopEngine(){
  rightEngine(0, 0, 0);
  leftEngine(0, 0, 0);
}
