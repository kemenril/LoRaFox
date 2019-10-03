#ifndef __MORSE_H
#define __MORSE_H

//In multiples of dot length.
#define DASH_MULT      3
#define SPACE_MULT     3
//Subtract the letter space from the following, so one word can smoothly follow the next.
//One unit of space is automatically added to the end of a Morse symbol, so you don't need quite as much as you think.
#define LTRSPACE_MULT  5
#define WORDSPACE_MULT 11

const char* mchar_A = ".-";
const char* mchar_B = "-...";
const char* mchar_C = "-.-.";
const char* mchar_D = "-..";
const char* mchar_E = ".";
const char* mchar_F = "..-.";
const char* mchar_G = "--.";
const char* mchar_H = "....";
const char* mchar_I = "..";
const char* mchar_J = ".---";
const char* mchar_K = "-.-";
const char* mchar_L = ".-..";
const char* mchar_M = "--";
const char* mchar_N = "-.";
const char* mchar_O = "---";
const char* mchar_P = ".--.";
const char* mchar_Q = "--.-";
const char* mchar_R = ".-.";
const char* mchar_S = "...";
const char* mchar_T = "-";
const char* mchar_U = "..-";
const char* mchar_V = "...-";
const char* mchar_W = ".--";
const char* mchar_X = "-..-";
const char* mchar_Y = "-.--";
const char* mchar_Z = "..--";

//Numbers
const char* mchar_1 = ".----";
const char* mchar_2 = "..---";
const char* mchar_3 = "...--";
const char* mchar_4 = "....-";
const char* mchar_5 = ".....";
const char* mchar_6 = "-....";
const char* mchar_7 = "--...";
const char* mchar_8 = "---..";
const char* mchar_9 = "----.";
const char* mchar_0 = "-----";

//Other
const char* mchar_PERIOD = ".-.-.-";
const char* mchar_QUESTION = "..--..";
const char* mchar_SLASH = "-..-.";

const char* entity(char x) {
  //Note that this switch statement has no breaks, because the action is always to return immediately.
  switch (x) {
    case 'A':
      return mchar_A;
    case 'B':
      return mchar_B;
    case 'C':
      return mchar_C;
    case 'D':
      return mchar_D;
    case 'E':
      return mchar_E;
    case 'F':
      return mchar_F;
    case 'G':
      return mchar_G;
    case 'H':
      return mchar_H;
    case 'I':
      return mchar_I;
    case 'J':
      return mchar_J;
    case 'K':
      return mchar_K;
    case 'L':
      return mchar_L;
    case 'M':
      return mchar_M;
    case 'N':
      return mchar_N;
    case 'O':
      return mchar_O;
    case 'P':
      return mchar_P;
    case 'Q':
      return mchar_Q;
    case 'R':
      return mchar_R;
    case 'S':
      return mchar_S;
    case 'T':
      return mchar_T;
    case 'U':
      return mchar_U;
    case 'V':
      return mchar_V;
    case 'W':
      return mchar_W;
    case 'X':
      return mchar_X;
    case 'Y':
      return mchar_Y;
    case 'Z':
      return mchar_Z;
    case '0':
      return mchar_0;
    case '1':
      return mchar_1;
    case '2':
      return mchar_2;
    case '3':
      return mchar_3;
    case '4':
      return mchar_4;
    case '5':
      return mchar_5;
    case '6':
      return mchar_6;
    case '7':
      return mchar_7;
    case '8':
      return mchar_8;
    case '9':
      return mchar_9;
    case '.':
      return mchar_PERIOD;
    case '?':
      return mchar_QUESTION;
    case '/':
      return mchar_SLASH;
    case ' ':
      return " "; //Just pass the space through.
    default:  //Simply ignore characters not in the table.
      return "";
    }
}

#endif
