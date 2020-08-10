#include<iostream>

using namespace std;

// Fn declarations
char *generateRandomKey(int stringLength);
int getSectionSum(char *section, int start, int end);
bool isValidSection(char *section, int start, int end, int divisor);
char *makeSectionValid(char *section, int start, int end, int divisor, int protectedIndex);
char *removeBadAsciiChars(char *key, int len);

// Const declarations
const char minAsciiDecimalValue = 65; // 'A'
const char maxAsciiDecimalValue = 122; // 'z'
const int minBadAsciiRange = 91; 
const int maxBadAsciiRange = 96;
const int divisors[] = { 3, 4, 5, 4 };
const int protectedIndicies[] = { 2, -1, -1, 13 };
const int stringLength = 16;

int main() {
  char *key = removeBadAsciiChars(generateRandomKey(stringLength), stringLength);

  for (int i = 0; i < stringLength; i += 4) {
    int divisor = divisors [i / 4];
    int protectedIndex = protectedIndicies[i / 4];
    char *section = makeSectionValid(key, i, i + 4, divisor, protectedIndex);
    for (int j = i; j < i + 4; j++) {
      key[j] = section[j];
    }
  }
  cout << key << endl;
  return 0;
}

char *generateRandomKey(int stringLength) {
  char *key = new char[stringLength];
  srand(time(NULL));
  for (int i = 0; i < stringLength; i++) {
    // rand val between [ minAscii, maxAscii ]
    key[i] = minAsciiDecimalValue + ( rand() % ( maxAsciiDecimalValue - minAsciiDecimalValue + 1) );
  }
  key[2] = 'B';
  key[13] = 'Q';
  return key;
}

int getSectionSum(char *section, int start, int end) {
  int sum = 0;
  for (int i = start; i < end; i++) {
    sum += section[i];
  }
  return sum;
}

bool isValidSection(char *section, int start, int end, int divisor) {
  return getSectionSum(section, start, end) % divisor == 0;
}

char *makeSectionValid(char *section, int start, int end, int divisor, int protectedIndex) {
  int index = start;
  protectedIndex = protectedIndex % 4;

  while (!isValidSection(section, start, end, divisor)) {
    // Don't move into non A-z chars and avoid modifying protected indices ('B' and 'Q')
    if (section[index] == maxAsciiDecimalValue) {
      index++;
      if (index == protectedIndex) {
        index++;
      }
    }
    section[index]++;
  }
  return section;
}

char *removeBadAsciiChars(char *key, int len) {
  for (int i = 0; i < len; i++) {
    // Subtract 5 b/c then we don't run the risk of incrementing
    // a character into the bad range when inside makeSectionValid( ... )
    // b/c the largest divisor is 5
    if (key[i] >= minBadAsciiRange - 5 && key[i] <= maxBadAsciiRange) {
      key[i] = maxBadAsciiRange + 1;
    }
  }
  return key;
}
