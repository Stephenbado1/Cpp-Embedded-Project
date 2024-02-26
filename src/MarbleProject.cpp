#include "mbed.h"
#include <cmath>
#include "SevenSegmentSerial.h"
#include "Servo.h"

//declare hole sensors, servos, piezo buzzer, and score/attempts displays
InterruptIn sensor1(p18), sensor2(p17), sensor3(p16), sensor4(p15), sensor5(p14);
Servo servo1(p22), servo2(p23), servo3(p24), servo4(p25), servo5(p26);
PwmOut buzzer(p21);
BusOut displayMove (p6, p7, p8, p9, p10, p11, p12, p13);
SevenSegmentSerial displayScore(UART_MODE, p28);

//bool variables to indicate if each score hole has been mad yet and final victory/loss condition
bool numSensors[] = {false, false, false, false, false};
bool victoryFlag;
bool loseFlag;

//debugging LEDS
DigitalOut led(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

//declare hole sensors, array for servo positions to move through, and char array for attempt display
int sensors[] = {sensor1, sensor2, sensor3, sensor4, sensor5};
float positions[] ={0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};
char num[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90}; //0 - 9

//declare score and attempts variables
int totalScore = 0;
int currentMove = 8;
Timer debounceTimer;
Timer updateDisplay;

//victory/loss conditions with appropriate display
void displayString() {
    if (victoryFlag) {
        displayScore.write("GOOD");
        wait(1);
        displayScore.write("JOB");
        wait(1);
    } else {
        displayScore.write("YOU");
        wait(1);
        displayScore.write("LOSE");
        wait(1);
    }
}

//functions that plays different tone for each sensor and adds hole score to total
void startSong() {
    int freqs[] = {493, 987, 739, 622, 987, 0, 659, 0, 
                    523, 1046, 783, 622, 1046, 0, 659, 0,  
                    493, 987, 739, 622, 987, 0, 659, 0
    };
    for (int i = 0; i < sizeof(freqs) / sizeof(freqs[0]); i++) {
        buzzer.period(1.0 / freqs[i]);
        buzzer = 0.5;
        wait_us(55000);
        buzzer = 0.0;
        wait_us(55000);
    }
    int fastFreqs[] = {659, 659, 784, 0, 784, 830, 880, 0, 880, 932, 987, 0};
    for (int i = 0; i < sizeof(fastFreqs) / sizeof(fastFreqs[0]); i++) {
        buzzer.period(1.0 / fastFreqs[i]);
        buzzer = 0.5;
        wait_us(27500);
        buzzer = 0.0;
        wait_us(27500);
    }
    buzzer.period(1.0 / 1026);
    buzzer = 0.5;
    wait_us(110000);
    buzzer = 0.0;
    wait_us(55000);
    buzzer.period(.02);
}

void victorySong() {
    float freqs1[] = {523.251, 523.251, 523.251};
    for (int i = 0; i < sizeof(freqs1) / sizeof(freqs1[0]); i++) {
        buzzer.period(1.0 / freqs1[i]);
        buzzer = 0.5;
        wait_us(150000);
        buzzer = 0.0;
        wait_us(37500);
    }
    float freqs2[] = {523.251, 415.305, 466.164, 523.251, 0, 466.164, 523.251, 0, 0};
    
    for (int i = 0; i < sizeof(freqs2) / sizeof(freqs2[0]); i++) {
        if (i == 3 || i == 4) {
            buzzer.period(1.0 / freqs2[i]);
            buzzer = 0.5;
            wait_us(150000);
            buzzer = 0.0;
            wait_us(37500);
            i++;
        } else if (i == 5) {
            buzzer.period(1.0 / freqs2[i]);
            buzzer = 0.5;
            wait_us(150000);
            buzzer = 0.0;
            wait_us(37500);
            i++;
        }
        buzzer.period(1.0 / freqs2[i]);
        buzzer = 0.5;
        wait_us(300000);
        buzzer = 0.0;
        wait_us(37500);
    }
    
    float freqs3[] = {415.305, 523.251, 440, 349.228, 440, 0, 440, 523.251, 440, 349.228, 440, 466.164, 523.251, 466.164, 440, 349.228, 
                    440, 523.251, 440, 349.228, 440, 0, 440, 523.251, 440, 349.228, 440, 466.164, 523.251, 466.164, 440, 349.228};
    for (int i = 0; i < sizeof(freqs3) / sizeof(freqs3[0]); i++) {
        buzzer.period(1.0 / freqs3[i]);
        buzzer = 0.5;
        wait_us(75000);
        buzzer = 0.0;
        wait_us(37500);
    }
    buzzer.period(.02);
}

void losingSong() {
    buzzer.period(1 / 500.5);
    buzzer = 0.5;
    wait_us(500000);
    buzzer = 0.0;

    displayString();
    currentMove = 8;
    totalScore = 0;
    for (int i = 0; i < sizeof(numSensors) / sizeof(numSensors[0]); i++) {
        numSensors[i] = false;
    }
    buzzer.period(.02);
}

void checkVictory() {
    //displayScore = totalScore;
    for (int i = 0; i < sizeof(numSensors) / sizeof(numSensors[0]); i++) {
        if (!numSensors[i]) {
            //checks if every hole has been scored
            return;
        }
    }
    //getting all holes. resets game
    victoryFlag = true;
    victorySong();
    displayString();
    victoryFlag = false;
    currentMove = 8;
    totalScore = 0;
    for (int i = 0; i < sizeof(numSensors) / sizeof(numSensors[0]); i++) {
        numSensors[i] = false;
    }
}

void marioCoin() {
    if (debounceTimer.read_us() > 2000000) {
        debounceTimer.reset();
        --currentMove;

        buzzer.period(1 / (987.767));
        buzzer = 0.5;
        wait_us(55000);
        buzzer = 0.0;
        buzzer.period(1 / (1318.51));
        buzzer = 0.5;
        wait_us(500000);
        buzzer = 0.0;
        wait_us(250000);

        totalScore = totalScore + 50;

        numSensors[0] = true;
        checkVictory();
    }
    buzzer.period(.02);
}

void itemGet() {
    if (debounceTimer.read_us() > 2000000) {
        debounceTimer.reset();
        --currentMove;

        float freqs[] = {391.995, 440, 466.164};
        for (int i = 0; i < sizeof(freqs) / sizeof(freqs[0]); i++) {
            buzzer.period(1.0 / freqs[i]);
            buzzer = 0.5;
            wait_us(160000);
        };
        buzzer.period(1.0 / 493.883);
        buzzer = 0.5;
        wait_us(500000);
        buzzer = 0.0;
        wait_us(250000);

        totalScore += 50;

        numSensors[1] = true;
        checkVictory();
    }
    buzzer.period(.02);
}

void sonicRing() {
    if (debounceTimer.read_us() > 2000000) {
        debounceTimer.reset();
        --currentMove;

        float freqs[] = {1318.51, 1567.98};
        for (int i = 0; i < sizeof(freqs) / sizeof(freqs[0]); i++) {
            buzzer.period(1.0 / freqs[i]);
            buzzer = 0.5;
            wait_us(50000);
        };
        buzzer.period(1.0 / 2093);
        buzzer = 0.5;
        wait_us(500000);
        buzzer = 0.0;
        wait_us(250000);

        totalScore += 50;

        numSensors[2] = true;
        checkVictory();
    }
    buzzer.period(.02);
}

void pokemonSound() {
    if (debounceTimer.read_us() > 2000000) {
        debounceTimer.reset();
        --currentMove;

        float freqs[] = {450, 450, 450};
        for (int i = 0; i < sizeof(freqs) / sizeof(freqs[0]); i++) {
            buzzer.period(1.0 / freqs[i]);
            buzzer = 0.5;
            wait_us(75000);
            buzzer = 0.0;
            wait_us(37500);
        };
        buzzer.period(1.0 / 900);
        buzzer = 0.5;
        wait_us(500000);
        buzzer = 0.0;
        wait_us(250000);

        totalScore += 50;

        numSensors[3] = true;
        checkVictory();
    }
    buzzer.period(.02);
}

void snakeAlert() {
    if (debounceTimer.read_us() > 2000000) {
        debounceTimer.reset();
        --currentMove;

        float freqs[] = {277.183, 329.628, 391.995, 468.164, 554.365, 659.254, 783.991};
        for (int i = 0; i < sizeof(freqs) / sizeof(freqs[0]); i++) {
            buzzer.period(1.0 / freqs[i]);
            buzzer = 0.5;
            wait_us(10000);
            buzzer = 0.0;
        };
        buzzer.period(1.0 / 932.328);
        buzzer = 0.5;
        wait_us(500000);
        buzzer = 0.0;
        wait_us(250000);

        totalScore += 50;

        numSensors[4] = true;

        checkVictory();
    }
    buzzer.period(.02);
}

//preset servo posiitons to optimize the chance that each hole is made
void hole1() {
    buzzer.period(0.5);
    servo1 = positions[3];
    servo2 = positions[3];
    servo4 = positions[5];
    buzzer.period(0.0);
}

void hole2() {
    buzzer.period(0.5);
    servo1 = positions[2];
    servo2 = positions[9];
    servo4 = positions[1];
    buzzer.period(0.0);
}

void hole3() {
    buzzer.period(0.5);
    servo1 = positions[2];
    servo2 = positions[7];
    servo4 = positions[8];
    buzzer.period(0.0);
}

void hole4() {
    buzzer.period(0.5);
    servo1 = positions[5];
    servo3 = positions[10];
    servo5 = positions[0];
    buzzer.period(0.0);
}

void hole5() {
    buzzer.period(0.5);
    servo1 = positions[5];
    servo3 = positions[6];
    servo5 = positions[7];
    buzzer.period(0.0);
}

//update display while game is running
void displayUpdate(){
    if (updateDisplay.read() > 1) {
        if (currentMove > 0) {
            displayMove = num[currentMove];
            displayScore = totalScore;
            updateDisplay.reset();   
        } else {
            losingSong();
        }
        //wait(1);
        updateDisplay.reset();
    }
}

//main method runs a loop checking each holes made/not made status and keeps track of score/attempts
int main() {
    displayScore = totalScore;
    displayMove = num[currentMove];
    startSong();

    //starts timer to avoid the machine from making sounds all the time
    debounceTimer.start();
    updateDisplay.start();
    
    //set buzzer sounds from each sensor interrupt
    sensor1.rise(&marioCoin);
    sensor2.rise(&itemGet);
    sensor3.rise(&sonicRing);
    sensor4.rise(&pokemonSound);
    sensor5.rise(&snakeAlert);

    //once a hole is made servos switch to next hole that has not been made yet(can be made out of order)
    while (1) {
        if (!numSensors[0]) {
            hole1();
            displayUpdate();
            wait(0.01);
        }else if (!numSensors[1]) {
            hole2();
            displayUpdate();
            wait(0.01);
        }else if (!numSensors[2]) {
            hole3();
            displayUpdate();
            wait(0.01);
        }else if (!numSensors[3]) {
            hole4();
            displayUpdate();
            wait(0.01);
        }else if (!numSensors[4]) {
            hole5();
            displayUpdate();
            wait(0.01);
        }
    }
}
