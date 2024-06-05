#ifndef buffer_hpp
#define buffer_hpp
#include <iostream>
using namespace std;

const char eofChar = '$';

class Buffer {
    private:
        string buffer;
        int inPos;
    public:
        Buffer() {

        }
        void init(string& str) {
            buffer = str;
            inPos = 0;
        }
         char getChar() {
            return inPos < buffer.size() ? buffer[inPos]:eofChar;
        }
        char advance() {
            if (inPos+1 < buffer.size()) {
                inPos++;
                return buffer[inPos];
            }
            inPos++;
            return eofChar;
        }
        char reverse() {
            if (inPos-1 > 0) {
                inPos--;
            }
            return buffer[inPos];
        }
        bool isEOF() {
            return inPos >= buffer.size();
        }
};

#endif