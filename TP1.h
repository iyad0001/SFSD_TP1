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

// record = <ccp account, balance>
typedef struct Ccp {
    int ccp_number;
    int balance;
} CCP;

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

// gets wilaya name given its number
char *getWilayaName(int wilaya_num);

// creates a random weather measurements file named fname of size num_recs and returns FILE*
FILE *createRandomFile(const char *fname, int num_recs);

// creates an empty weather measurements file (returns NULL if file is not found)
FILE *createEmptyFile(const char *fname);

// opens an existing weather measurements file (returns NULL if file is not found)
FILE *openExistingFile(const char *fname);

// inserts a weather measurement record at the end of the file
// must check that buffer buf is valid before function call
void insert(FILE *f, t_rec1 buf);

// modifies the temperature of a given wilaya on a given date
// returns 0 on success, non-zero if record not found
int modifyTemp(const char *wilaya, const char *date, float temp, FILE *f);

// deletes all records of a given wilaya
// returns a new FILE* opened on the updated file (or NULL on error)
FILE *deleteWilaya(const char *wilaya, FILE *f, char *filename);

// prints min, max temperatures of a given wilaya with their corresponding dates
// and prints average temperature of the given wilaya
void wilayaStats(const char *wilaya, FILE *f);

// prints weather mesurements file
void printFile(FILE *f);

// encodes a given file using a key of n bytes
FILE *encode(FILE *f, char *fname_encoded, const char *key, int n);

// decodes a given file using a key of n bytes
FILE *decode(FILE *f, char *fname_decoded, const char *key, int n);

/* --- Menu Functions --- */

// main menu function
void mainMenu();

// prints temperature measurements menu
void menu1();

// Create/open file menu
void menu1_1(char *filename, FILE **f);

// Functions on file (insert, modify, delete, stats)
void menu1_3(FILE **f);

// Encode/Decode file
void menu1_4(FILE **f, char *filename_encoded, char *filename_decoded, FILE **f_encoded, FILE **f_decoded);

/* --- CCP (MENU 2) --- */

// create CCP accounts binary file with consecutive ccp numbers starting at min_ccp_number
FILE* create_random_file1(char *file_name_1, int number_of_accounts , int min_ccp_number );

// search a CCP account in accounts binary file (returns 1 if found, 0 otherwise)
int search_ccp(FILE *file1, int ccp_number);

// create transfers text file with random transfers (each line: "<ccp_number> <amount>\n")
FILE* create_random_file2(char *file_name, int number_of_transfers, int min_ccp_number, int number_of_accounts);

// insert a transfer line into transfers file (text)
void insert_transfer(FILE *file1, FILE *file2, int ccp_number, int balance);

// insert an account (binary) if not present
void insert_account(FILE *file1, int ccp_number, int balance);

// apply transfers from transfers file to accounts file (update balances)
void update (FILE *file1, FILE *file2);

// menus for CCP management - pass file pointers by address so menu can open/close files
void menu2 ();
void menu2_1(FILE **file1, FILE **file2) ;
void menu2_2(FILE **file1, FILE **file2);
void menu2_3 (FILE **file1, FILE **file2);

int printfile_CCP (FILE *file1);
int printfile_transfers(FILE *file2);
int search_transfer(FILE *file2, int ccp_number);

#endif