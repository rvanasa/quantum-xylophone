#ifndef QUAXY_CPP_REGISTER_HPP
#define QUAXY_CPP_REGISTER_HPP


template<int N>
class Register {
private:
    complex<float> _state[1 << N];
    char _modifier;
    int _modifierIndex;
    int _error;

    void _apply(int index, int depth, int section,
                complex<float> a, complex<float> b, complex<float> c, complex<float> d) {
        int depthSize = 1 << depth;
        if(index == depth) {
            int width = size() / depthSize;
            int half = width / 2;

            int left = section * width;
            int middle = left + half;

            for(int i = left; i < middle; i++) {
                auto x = _state[i];
                auto y = _state[i + half];
                _state[i] = a * x + b * y;
                _state[i + half] = c * x + d * y;
            }
        }
        else {
            _apply(index, depth + 1, section, a, b, c, d);
            _apply(index, depth + 1, section + depthSize, a, b, c, d);
        }
    }

public:
    Register() {
        reset();
    }

    void reset() {
        for(int i = 0; i < size(); i++) {
            _state[i] = i ? 0 : 1;
        }
        _modifier = 0;
        _modifierIndex = 0;
        _error = 0;
    }

    int qubitCount() const {
        return N;
    }

    int size() const {
        return 1 << N;
    }

    complex<float> &view(int i) {
        return _state[i];
    }

    bool hasModifier() {
        return _modifier;
    }

    int consumeError() {
        int error = _error;
        _error = 0;
        return error;
    }

//    complex<float>[1 << N] &viewState() {
//        return _state;
//    }

    int measure() {
        int len = size();
        // `sum` should always be 1, but calculating anyway for robustness
        float sum = 0;
        for(int i = 0; i < len; i++) {
            float mag = norm(_state[i]);
            sum += mag;
        }
        int measureIndex = 0;
        if(sum != 0) {
            // Weighted random
            float r = randomFloat(sum);
            for(int i = 0; i < len; i++) {
                float mag = norm(_state[i]);
                if(mag > r) {
                    measureIndex = i;
                    break;
                }
                r -= mag;
            }
        }
        for(int i = 0; i < len; i++) {
            _state[i] = i == measureIndex ? 1 : 0;
        }
        return measureIndex;
    }

    int measure(int index, bool preserveState = false) {
        swap(index, 0);

        float leftSum = 0;
        float rightSum = 0;
        int half = size() / 2;
        for(int i = 0; i < half; i++) {
            leftSum += norm(_state[i]);
            rightSum += norm(_state[i + half]);
        }
        int result = -1;
        if(leftSum != 0 || rightSum != 0) {
            result = randomFloat(leftSum + rightSum) < rightSum;
//            cout<<'['<<result<<']';

            if(!preserveState) {
                float mag = sqrt(result ? rightSum : leftSum);
                if(mag > 0) {
                    for(int i = 0; i < half; i++) {
                        _state[result ? i : i + half] = 0;
                        _state[result ? i + half : i] /= mag;
                    }
                }
            }
        }

        swap(index, 0);
        return result;
    }

    bool apply(int index, char gate) {
        if(index < 0 || index >= qubitCount()) {
            return false;
        }
        else if(gate == 'S' && _modifier == 'S') {
            return swap(_modifierIndex, index);
        }
        else if(gate == 'C' || gate == 'S') {
            _modifier = _modifier == gate ? 0 : gate;
            _modifierIndex = index;
            return true;
        }
        else if(gate == 'M') {
            measure(index);
            return true;
        }
        else if(gate == 'X') {
            return apply(index, 0, 1, 1, 0);
        }
        else if(gate == 'Y') {
            return apply(index, 0, complex<float>(0, -1), complex<float>(0, 1), 0);
        }
        else if(gate == 'Z') {
            return apply(index, 1, 0, 0, -1);
        }
        else if(gate == 'H') {
            auto f = (float) (1 / sqrt(2));
            return apply(index, f, f, f, -f);
        }
        else {
            return false;
        }
    }

    bool apply(int index, complex<float> a, complex<float> b, complex<float> c, complex<float> d) {
        char modifier = _modifier;
        _modifier = 0;
        if(modifier == 'C') {
            if(_modifierIndex != index) {
                _controlSwap(index, false);
                _apply(1, 1, 1, a, b, c, d);
                _controlSwap(index, true);
                return true;
            }
        }
        else {
            swap(index, 0);
            _apply(0, 0, 0, a, b, c, d);
            swap(index, 0);
            return true;
        }
        return false;
    }

    void _controlSwap(int index, bool reverse) {
        if((index == 0) == reverse) {
            if(index != 0 || _modifierIndex != 1) {
                swap(_modifierIndex, 0);
            }
            swap(index, 1);
        }
        else {
            swap(index, 1);
//            swap(_modifierIndex, 0);
            if(index != 0 || _modifierIndex != 1) {
                swap(_modifierIndex, 0);
            }
        }
    }

    bool swap(int a, int b) {
        if(a == b) {
            return false;
        }
        // Reverse bit order
        a = N - 1 - a;
        b = N - 1 - b;

        complex<float> copy[1 << N];
        int len = size();
        for(int i = 0; i < len; i++) {
            copy[i] = _state[i];
        }
        for(int i = 0; i < len; i++) {
            int flags = (((i >> a) & 1) ^ ((i >> b) & 1));
            int j = (i ^ ((flags << a) | (flags << b)));
            _state[i] = copy[j];
        }
        return true;
    }
};

#endif
