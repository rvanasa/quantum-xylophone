

class Operator {
  private:
    char _name;
  
  public:
    Operator(char name) {
      _name = name;
    }

    char getName() const {
      return _name;
    }
  
    void reset() {
    }
  
    virtual void on(int index, Register &reg) = 0;
};


class PlaceholderOperator : public Operator {
  public:
    PlaceholderOperator(char name) : Operator(name) {
    }

    void on(int index, Register &reg) {
    }
};


class QubitOperator : public Operator {
  private:
    Matrix2cf _unitary;

  public:
    QubitOperator(char name, Matrix2cf unitary) : Operator(name) {
      _unitary = unitary;
    }

    QubitOperator(char name, complex<float> a, complex<float> b, complex<float> c, complex<float> d) : QubitOperator(name, Matrix2cf()) {
      _unitary << a, b, c, d;
    }
  
    void on(int index, Register &reg) {
      QubitGate(index, _unitary).apply(reg);
    }
};


class MeasureOperator : public Operator {
  public:
    MeasureOperator(char name) : Operator(name) {
    }
  
    void on(int index, Register &reg) {
      MeasureGate(index).apply(reg);
    }
};


class ModifierOperator : public Operator {
  public:
    ModifierOperator(char name) : Operator(name) {
    }
  
    void on(int index, Register &reg) {
//      Modifier modifier = createModifier(index);
      //
    }

//    virtual Modifier createModifier(int index) = 0;
};


class ControlledOperator : public ModifierOperator {
  public:
    ControlledOperator(char name) : ModifierOperator(name) {
    }

    void on(int index, Register &reg) {
       
    }
  
//    Modifier createModifier(int index) {
//      ControlledModifier modifier = new ControlledModifier(index);
//      return modifier;
//    }
};
