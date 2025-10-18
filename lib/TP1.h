#ifndef TP1_H
#define TP1_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

// ---- Structs ----
// record = <wilaya, date, temp> (Weather measurements)
typedef struct T_rec1 {
    char wilaya[25];
    char date[11];
    float temp;
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
#define ERR_FUTURE_DATE 0x20


// ---- Function prototypes ----

// 1. Weather mesurements

// helper functions

// returns ERR_NONE (0) if date is valid, otherwise it returns the corresponding error number
int isValidDate(const char *date);

// returns a random integer in interval [low, up]
int randomIn(int low, int up);

// creates a random weather measurements file named fname of size num_recs and returns FILE*
FILE *createRandomFile(const char *fname, int num_recs);

// creates an empty weather measurements file (returns NULL if file is not found)
FILE *createEmptyFile(const char *fname);

// opens an existing weather measurements file (returns NULL if file is not found)
FILE *openExistingFile(const char *fname);

// inserts a weather measurement record at the end of the file
// must check that buffer buf is valid before function call
void insert(FILE *f, t_rec1 *buf);

// modifies the temperature of a given wilaya on a given date
// returns 0 on success, non-zero if record not found
int modifyTemp(const char *wilaya, const char *date, float temp, FILE *f);

// deletes all records of a given wilaya
// returns a new FILE* opened on the updated file (or NULL on error)
FILE *deleteWilaya(const char *wilaya, FILE *f);

// prints min, max temperatures of a given wilaya with their corresponding dates
// and prints average temperature of the given wilaya
void wilayaStats(const char *wilaya, FILE *f);

// prints weather mesurements file
void printFile(FILE *f);






#endif