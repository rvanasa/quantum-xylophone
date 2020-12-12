// https://arduino.stackexchange.com/questions/67239/tone-conflicts-with-irremote-library-multiple-definition-of-vector-7
#include <IRremote.h>

#include <stlport.h>
#include <Eigen30.h>

using namespace std;

float randomFloat(float max) {
  return random(1000) * max / 1000;
}

#include "quaxy-cpp/Register.hpp"


#define BASE_RESISTANCE 220
#define VOLTAGE_TOLERANCE 0.3

#define GATE_DEBOUNCE 1000
#define GATE_CONFIRMATION_COUNT 5

#define MEASURE_DURATION 1000
#define SOUND_DURATION 200

#define REMOTE_PIN 5
#define BUZZER_PIN 6
#define ZERO_PIN 2
#define ONE_PIN 3
#define MODIFIER_PIN 13

// https://easyeda.com/winter2015/Infrared_IR_Remote-DqnTNjd7f
IRrecv remote(REMOTE_PIN);
decode_results results;
int remoteIndex = -1;

#define QUBIT_RESET_BUTTON 0xFFA25D // POWER
#define QUBIT_DESELECT_BUTTON 0xFFE21D // FUNC/STOP
#define QUBIT_SAVE_BUTTON 0xFFE01F // DOWN
#define QUBIT_LOAD_BUTTON 0xFF906F // UP

const unsigned long QUBIT_BUTTONS[] = {0xFFC23D, 0xFF629D, 0xFF22DD, 0xFFA857, 0xFF02FD};
const unsigned long NUMBER_BUTTONS[] = {0xFF6897, 0xFF30CF, 0xFF18E7, 0xFF7A85, 0xFF10EF, 0xFF38C7, 0xFF5AA5, 0xFF42BD, 0xFF4AB5, 0xFF52AD};

#include "Notes.hpp"

const pair<pair<int, char>, int> OPERATORS[] = {
  {{30, '?'}, 0},
  {{100, 'X'}, NOTE_C4},
  {{220, 'Y'}, NOTE_D4},
  {{330, 'Z'}, NOTE_E4},
  {{1000, 'H'}, NOTE_F4},
  {{2000, 'C'}, NOTE_G4},
  {{5100, 'S'}, NOTE_A4},
  {{10000, 'M'}, NOTE_B4},
};

unsigned long long loopCount = 0;
int measurement = -1;
int measureDuration = 0;
int soundDuration = 0;
int cascadeIndex = -1;

#include "QubitWatcher.hpp"
#include "Remote.hpp"

const QubitWatcher QUBIT_WATCHERS[] = {
  QubitWatcher(A0, 8, NOTE_CS4),
  QubitWatcher(A1, 9, NOTE_DS4),
  QubitWatcher(A2, 10, NOTE_FS4),
  QubitWatcher(A3, 11, NOTE_GS4),
  QubitWatcher(A4, 12, NOTE_AS4),
};
// QUBIT_COUNT == sizeof(QUBIT_WATCHERS) / sizeof(QUBIT_WATCHERS[0])


void setup() {
  Serial.begin(9600);

  remote.enableIRIn();
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(ZERO_PIN, OUTPUT);
  pinMode(ONE_PIN, OUTPUT);
  pinMode(MODIFIER_PIN, OUTPUT);
  for(auto watcher : QUBIT_WATCHERS) {
    watcher.setup();
  }

  updateInterface();

//  Serial.println("-----");
//  for(int r : gateResistances) {
//    Serial.println(100/(1 + 100/(double)r));
//  }
//  Serial.println("-----");


//  Serial.println("\n-----\n");
//  auto state = reg.viewState();
//  for(int i = 0; i < state.cols(); i++) {
//    Serial.print(state(i).real());
//    if(state(i).imag()) {
//      Serial.print('+');
//      Serial.print(state(i).imag());
//      Serial.print('i');
//    }
//    Serial.println();
//  }
//  Serial.println("\n-----\n");
}


void loop() {
  if(remote.decode(&results)) {
    if(results.value != -1) {
      Serial.print("0x");/////
      Serial.println(results.value, HEX);/////
      
      if(results.value == QUBIT_RESET_BUTTON) {
        Serial.println("Reset");
        reg.reset();
        playSound(NOTE_B0);
      }
      else if(results.value == QUBIT_DESELECT_BUTTON) {
        Serial.println("Deselect");
        remoteIndex = -1;
        playSound(NOTE_C2);
      }
      else if(results.value == QUBIT_SAVE_BUTTON) {
        Serial.println("Save");
        save();
        playSound(NOTE_A5);
      }
      else if(results.value == QUBIT_LOAD_BUTTON) {
        Serial.println("Load");
        load();
        playSound(NOTE_E6);
      }
      else {
        int i = 0;
        for(auto button : QUBIT_BUTTONS) {
          if(results.value == button) {
            Serial.print(":: ");
            Serial.println(i);
            remoteIndex = i;
            playSound(QUBIT_WATCHERS[i].sound);
          }
          i++;
        }
        
        i = 0;
        for(auto [ab, _] : OPERATORS) {
          auto [__, code] = ab;
          auto button = NUMBER_BUTTONS[i];
          if(results.value == button) {
            if(remoteIndex == -1) {
              for(int j = 0; j < QUBIT_COUNT; j++) {
                applyOperator(j, code, true);
              }
              // TODO: measure all qubits together
//              for(int j = 0; j < QUBIT_COUNT; j++) {
//                Serial.print(reg.measure(j, true));
//              }
              Serial.println();
              updateInterface();
            }
            else {
              applyOperator(remoteIndex, code);
            }
          }
          i++;
        }
      }
    }
    remote.resume();
  }
  
  for(int index = 0; index < QUBIT_COUNT; index++) {
    QUBIT_WATCHERS[index].update();
  }
  
  delayMicroseconds(100);
  loopCount += 1;

  if(measureDuration > 0 && --measureDuration == 0) {
    measurement = -1;
    updateInterface();
  }
  
  if(soundDuration > 0 && --soundDuration == 0) {
    if(cascadeIndex != -1) {
      if(cascadeIndex >= QUBIT_COUNT) {
        cascadeIndex = -1;
        playSound(0); // Reset buzzer
      }
      else {
        // TODO: refactor pause logic
        noTone(BUZZER_PIN);
        delay(100);
        playMeasureSound(cascadeIndex);
        cascadeIndex++;
      }
    }
    else {
      playSound(0); // Reset buzzer
    }
  }
}

void applyOperator(int index, char code) {
  applyOperator(index, code, false);
}

// Apply a quantum operator
void applyOperator(int index, char code, bool multi) {
  Serial.print(index);
  Serial.print(" ");
  Serial.println(code);

  bool defyingReality = code == '?';
  
  reg.apply(index, code);
  if(!multi) {
    if(defyingReality || code == 'M') {
      measurement = reg.measure(index, true);
      measureDuration = MEASURE_DURATION;
      if(defyingReality) {
        playMeasureSound(index, measurement);
      }
    }
    updateInterface();
  }
  else if(defyingReality) {
    playMeasureSounds();
  }
  
  if(!defyingReality) {
    // Find the sound for this operator
    int sound = QUBIT_WATCHERS[index].sound;
    for(auto [ab, s] : OPERATORS) {
      auto [_, c] = ab;
      if(code == c && s != 0) {
        sound = s;
        break;
      }
    }
    playSound(sound);
  }
}

// Save the state of the register
void save() {
  regBackup = reg;
  updateInterface();
}

// Load the previous state of the register
void load() {
  reg = regBackup;
  updateInterface();
}

void playSound(int frequency) {
  playSound(frequency, SOUND_DURATION);
}

void playSound(int frequency, int duration) {
  soundDuration = duration;
  
  if(frequency > 0 && soundDuration > 0) {
    tone(BUZZER_PIN, frequency);
  }
  else {
    noTone(BUZZER_PIN);
  }
}

void playMeasureSounds() {
  cascadeIndex = 0;
  playMeasureSound(cascadeIndex);
}

void playMeasureSound(int index) {
  playMeasureSound(index, reg.measure(index, true));
}

void playMeasureSound(int index, int measurement) {
  playSound(measurement == 1 ? NOTE_F5 : measurement == 0 ? NOTE_D2 : 0);
}

// Update the physical interface
void updateInterface() {
  digitalWrite(ZERO_PIN, measurement == 0 ? HIGH : LOW);
  digitalWrite(ONE_PIN, measurement == 1 ? HIGH : LOW);
  
  digitalWrite(MODIFIER_PIN, reg.hasModifier() ? HIGH : LOW);
}
