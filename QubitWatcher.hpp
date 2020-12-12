const int QUBIT_COUNT = 5;

Register<QUBIT_COUNT> reg;
Register<QUBIT_COUNT> regBackup;

int _indexInc = 0;
class QubitWatcher {
  private:
    int _index;
    int _activeGate = -1;
    int _debounce = 0;
    int _confirmations = 0;
  public:
    int inputPin;
    int outputPin;
    int sound;

    QubitWatcher(int inputPin, int outputPin, int sound) {
      this->inputPin = inputPin;
      this->outputPin = outputPin;
      this->sound = sound;
      _index = _indexInc++;
    }

    void setup() {
      pinMode(inputPin, INPUT);
      pinMode(outputPin, OUTPUT);
      digitalWrite(outputPin, HIGH);
    }

    void update() {int sample = analogRead(inputPin);
      sample = 1023 - sample;///
  
  //    if(index == 0) Serial.println(sample);///////
  
//      if(loopCount % 1000 == 0) {
//        Serial.print("[");
//        Serial.print(_index);
//        Serial.print("] ");
//        Serial.println(sample);
//        if(_index == QUBIT_COUNT - 1) {
//          Serial.println();
//        }
//      }
      
      if(sample >= 1 && sample <= 1022) {
        double predicted = BASE_RESISTANCE * (sample / (1023. - sample));
        
        int i = 0;
        for(auto [ab, sound] : OPERATORS) {
          auto [resistance, code] = ab;
          double logDiff = 1 - resistance / predicted;
          if((i == 0 || logDiff >= -VOLTAGE_TOLERANCE) && logDiff <= VOLTAGE_TOLERANCE) {
            if(_activeGate == i) {
              if(_confirmations++ == GATE_CONFIRMATION_COUNT) {
                Serial.println(predicted);//////////////////////////////////
                applyOperator(_index, code);
              }
            }
            else if(_activeGate == -1) {
              _activeGate = i;
              _debounce = GATE_DEBOUNCE;
              _confirmations = 0;
            }
            else if(_activeGate < i && _confirmations > 0) {
              _confirmations--;
            }
            break;
          }
          i++;
        }
      }
  
      if(_debounce > 0 && --_debounce == 0) {
        _activeGate = -1;
      }
    }
};
