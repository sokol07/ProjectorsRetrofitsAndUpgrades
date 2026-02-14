//  maski bitowe okreslające kolejne kroki
byte patterns[8] = {
  0b00001000,
  0b00001100,
  0b00000100,
  0b00000110,
  0b00000010,
  0b00000011,
  0b00000001,
  0b00001001,
};
  unsigned long maxDelayLimit = 300000; // [us] okresla limit rpm min - generalny limit
  unsigned long currentMaxDelay = 300000; // [us] okresla rpm min wg potencjometru zakresu predkosci
  unsigned long minDelayLimit = 990; // [us] okresla rpm max - generalny limit
  unsigned long minDelay = 990; // [us] okresla rpm max wg potencjometru zakresu predkosci
  int sensorValue0 = 0; //  wartosc wejsciowa A0 (suwak)
  int speedPercent = 0; //  [%] procentowa wartosc predkosci z suwaka
  int smoothedSpeed = 0; // [%] wygladzona predkosc (do rampy rozpedzania)

  int sensorValue1 = 0; //  wartosc wejsciowa A1 (pokretlo)
  unsigned long calculatedDelay = 999999999;  // [us] rpm wyliczone do uzyskania obecnego rpm wg obu potencjometrow
  unsigned long previous_micros = 0;  // [us] timestamp poprzedniego cyklu wg 
  unsigned short i = 0; // iterator po krokach krokowca
  short step = 0; // krok wynikajacy z przelacznika kierunku

void setup() {
  // wejscie sterujace drivera silnika krokowego, kolejno IN1-IN4
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  // wybor kierunku
  pinMode(6, INPUT_PULLUP); // DI 6 - kierunek prawo
  pinMode(7, INPUT_PULLUP); // DI 7 - kierunek lewo
  //Serial.begin(115200);
}

void loop() {

  // odczyt i skalowanie suwaka A0
  sensorValue0 = analogRead(A0);
  speedPercent = map(sensorValue0, 0,1023,0,100);
  // odczyt i skalowanie pokretla A1 
  sensorValue1 = analogRead(A1);
  sensorValue1 = map(sensorValue1, 0,1023,1,10);

  // obliczenia
  minDelay = minDelayLimit*sensorValue1;
  if (sensorValue1==1){
    currentMaxDelay=10000;
  } else {
    currentMaxDelay = maxDelayLimit/((11-sensorValue1));
  }

  //wygladzanie predkosci (rampa) aby krokowiec ruszal prawidlowo przy gwaltownym starcie z maksymalna predkoscia
  if (step == 0) {
    smoothedSpeed = 0;
  }
  if (abs(smoothedSpeed-speedPercent)>10 && smoothedSpeed<speedPercent && step != 0){
        smoothedSpeed = smoothedSpeed+1;
  } else if (step != 0) {
        smoothedSpeed = speedPercent;
      }
  
  //wyliczenie opoznienia miedzy krokami silniak krokowego - z tego wynika predkosc obrotowa
  calculatedDelay = map(smoothedSpeed,1,100,currentMaxDelay,minDelay);

  //kierunek obrotów
  if (digitalRead(6)==false ) {
    step = 1;
  } else if (digitalRead(7)==false){
    step = -1;
  } else {
    step = 0;
  }

  // jesli od poprzedniego cyklu minal czas opoznienia (minus pol czasu cyklu zeby odswiezal prawidlowo przy najkrotszych delayach) - wykonaj kolejny krok silnika
  if (micros()-previous_micros>=calculatedDelay-(minDelayLimit/2)){
    if (speedPercent != 0 && step != 0){
      digitalWrite(2, bitRead(patterns[i], 0));
      digitalWrite(3, bitRead(patterns[i], 1));
      digitalWrite(4, bitRead(patterns[i], 2));
      digitalWrite(5, bitRead(patterns[i], 3));
      delayMicroseconds(2);
      i=i+step;
      if (i>255){
        i=7;
      }
      if (i>7){
        i=0;
      }
    } else {
      digitalWrite(2,0);
      digitalWrite(3,0);
      digitalWrite(4,0);
      digitalWrite(5,0);
      delay(10);
    }
    previous_micros = micros();
  }
}