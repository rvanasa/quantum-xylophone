#include <stlport.h>
#include <Eigen30.h>
#include <stack>

//#define EIGEN_RUNTIME_NO_MALLOC true

using namespace std;
using namespace Eigen;

#include "Helpers.hpp"
#include "Register.hpp"
#include "Modifier.hpp"
#include "Gate.hpp"
#include "Operator.hpp"


#define BASE_RESISTANCE 220
#define VOLTAGE_TOLERANCE 0.1

#define GATE_DEBOUNCE 1000
#define GATE_CONFIRMATION_COUNT 4

#define IMAG_UNIT complex<float>(0, 1)

const pair<int, Operator*> OPERATORS[] = {
  {30, new MeasureOperator('M')},
  {100, new QubitOperator('X', 0, 1, 1, 0)},
  {220, new QubitOperator('Y', 0, -IMAG_UNIT, IMAG_UNIT, 0)},
  {330, new QubitOperator('Z', 1, 0, 0, -1)},
  {1000, new QubitOperator('H', 1 / sqrt(2), 1 / sqrt(2), 1 / sqrt(2), -1 / sqrt(2))},
  {2000, new ControlledOperator('C')},
  {5100, new PlaceholderOperator('s')},
  {10000, new PlaceholderOperator('i')},
};

const int QUBIT_PINS[] = {
  A0,
  A1,
  A2,
  A3,
  A4,
};
const int QUBIT_COUNT = sizeof(QUBIT_PINS) / sizeof(QUBIT_PINS[0]);
//Register REGISTER(QUBIT_COUNT);

complex<float> STATE[2 << QUBIT_COUNT];

int activeGate = -1;
int activeDebounce = -1;
int confirmations = -1;


void setup() {
  Serial.begin(9600);

  for(int inputPin : QUBIT_PINS) {
    pinMode(inputPin, INPUT);
  }

  resetState();

//  Serial.println("-----");
//  for(int r : gateResistances) {
//    Serial.println(100/(1 + 100/(double)r));
//  }
//  Serial.println("-----");


  Serial.println(OPERATORS[1].second->getName());


  OPERATORS[1].second->on(0, REGISTER); // X on 0

//  Serial.println(REGISTER.viewState()(0).real());


  Serial.println("\n-----\n");
  auto state = REGISTER.viewState();
  for(int i = 0; i < state.cols(); i++) {
    Serial.print(state(i).real());
    if(state(i).imag()) {
      Serial.print('+');
      Serial.print(state(i).imag());
      Serial.print('i');
    }
    Serial.println();
  }
  Serial.println("\n-----\n");
}


void loop() {

  for(int index = 0; index < QUBIT_COUNT; index++) {
    int inputPin = QUBIT_PINS[index];
    
    int sample = analogRead(inputPin);
  
    int i;
    if(sample >= 1 && sample <= 1022) {
      double predicted = BASE_RESISTANCE * (sample / (1023. - sample));
      
      i = 0;
      for(auto [resistance, op] : OPERATORS) {
        double logDiff = 1 - resistance / predicted;
        if((i == 0 || logDiff >= -VOLTAGE_TOLERANCE) && logDiff <= VOLTAGE_TOLERANCE) {
          if(activeGate == i) {
            if(++confirmations == GATE_CONFIRMATION_COUNT) {
              
//              Serial.println(resistance);
//              Serial.println(op->getName());

              // Apply the operator to the current qubit index
              op->on(index, REGISTER);
            }
          }
          else if(activeGate == -1) {
            activeGate = i;
            activeDebounce = GATE_DEBOUNCE;
            confirmations = 0;
          }
          break;
        }
        i++;
      }
    }
    else {
      i = -1;
    }
  
    if(activeGate != i && activeDebounce > 0) {
      if(--activeDebounce == 0) {
        activeGate = -1;
      }
    }
  }
  
  delayMicroseconds(100);
}


void resetState() {
  STATE = {1};
}
