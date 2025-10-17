#ifndef TP1_H
#define TP1_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

// ---- Structs ----
// record = <wilaya, date, temp> (Weather measurements)
typedef struct T_rec1 {
    char *wilaya;
    char *date;
    int temp;
} t_rec1;

// record = <account, balance> (CCP)
typedef struct T_rec2 {
    int account;
    int balance;
} t_rec2;


// ---- Constants (macros) ----
// 1. error codes
// 1.1 date errors (as bit flags)
#define ERR_NONE        0x00
#define ERR_DAY_RANGE   0x01
#define ERR_MONTH_RANGE 0x02
#define ERR_YEAR_RANGE  0x04
#define ERR_LEAP_DAY    0x08
#define ERR_FORMAT      0x10


// ---- Function prototypes ----

// 1. Weather mesurements

// returns ERR_NONE (0) if date is valid, otherwise it returns the corresponding error number
int isValidDate(const char *date);

// returns a random integer in interval [low, up]
int random(int low, int up);








#endif