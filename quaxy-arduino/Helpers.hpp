
// Kronecker product of two matrices (https://en.wikipedia.org/wiki/Kronecker_product)
const MatrixXcf kronecker(MatrixXcf a, MatrixXcf b) {
  
  int rows = a.rows() * b.rows();
  int cols = a.cols() * b.cols();
  
  MatrixXcf result(rows, cols);

  for(int i = 0; i < rows; i++) {
    for(int j = 0; j < cols; j++) {
      result(i, j) = a(i / a.rows(), j / a.cols()) * b(i % b.rows(), j % b.cols());
    }
  }
  
  return result;
}


// freeMemory() :: remaining board memory (https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory)
#ifdef __arm__
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
