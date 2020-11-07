
#define MAX_HISTORY 10

struct History {
  int index;
  char name;
  RowVectorXcf previousState;
  History* previousHistory;
};

class Register {
  private:
    int _qubitCount;
    RowVectorXcf _state;
//    History* _history;
  
  public:

    Register(int qubitCount) {
      _qubitCount = qubitCount;

      reset();
    }

    void reset() {
      // 2^n possible states, where n is the number of qubits
      _state = RowVectorXcf::Zero(2 << _qubitCount);
      _state(0) = 1;
//      _history = NULL;
    }

    int qubitCount() const {
      return _qubitCount;
    }

    RowVectorXcf &viewState() {
      return _state;
    }

//    void setState(/*int index, char name, */RowVectorXcf state) {
//
//      Serial.println(freeMemory());
//
//      Serial.println("Changing");/////
//      
////      // Discard history beyond MAX_HISTORY steps
////      int i = 0;
////      History* node = _history;
////      while(node) {
////        History* current = node;
////        History* previous = node->previousHistory;
////        if(i++ >= MAX_HISTORY) {
////          current->previousHistory = NULL;
////          delete previous;
////          break;
////        }
////        else {
////          node = previous;
////        }
////      }
//
////      _history = new History{
////        .index = index,
////        .name = name,
////        .previousState = _state,
////        .previousHistory = _history
////      };
//      _state = state;
//
//      Serial.println("Changed");/////
//    }
};
