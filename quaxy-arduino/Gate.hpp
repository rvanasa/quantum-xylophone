
class Gate {
  private:

  public:
    virtual bool apply(Register &reg) = 0;
};


class IndexGate : public Gate {
  private:
    int _index;

  public:
    IndexGate(int index) {
      _index = index;
    }

    int getIndex() const {
      return _index;
    }
  
    bool apply(Register &reg) {
      int index = getIndex();
      if(index >= reg.qubitCount()) {
        return false;
      }
      return applyIndex(index, reg);
    }

    virtual bool applyIndex(int index, Register &reg) = 0;
};


class QubitGate : public IndexGate {
  private:
    Matrix2cf _unitary;

  public:
    QubitGate(int index, Matrix2cf unitary) : IndexGate(index) {
      _unitary = unitary;
    }
  
    bool applyIndex(int index, Register &reg) {
      MatrixXcf full = MatrixXcf::Ones(1, 1);
      for(int i = 0; i < reg.qubitCount(); i++) {
        full = kronecker(full, getIndex() == i ? _unitary : Matrix2cf::Identity());
      }
      
      Serial.println(":::");
      Serial.println(freeMemory());
      Serial.flush();

      auto state = reg.viewState();
      state.noalias() = full * state;

//      reg.setState(/*index, "(U)", */full * reg.viewState());

      return true;
    }
};


class MeasureGate : public IndexGate {
  public:
    MeasureGate(int index) : IndexGate(index) {
    }
  
    bool applyIndex(int index, Register &reg) {

      // TODO: only measure one qubit

      auto state = reg.viewState();
      
      float maxMag = 0;
      int maxIndex = 0;
      for(int i = 0; i < state.size(); i++) {
        float mag = norm(state(i));
        if(mag > maxMag) {
          maxMag = mag;
          maxIndex = i;
        }
      }

      for(int i = 0; i < state.size(); i++) {
        state(i) = i == maxIndex ? 1 : 0;
      }
      
//      reg.setState(index, "(M)", state);
    }
};
