#include "../lib/TP1.h"

// ---- Functions Implementation ----
// 1. weather mesurements

// helper functions

// returns ERR_NONE (0) if date is valid, otherwise it returns the corresponding error number
int isValidDate(const char *date) {
    int d, m, y;
    int pos = 0;
    int errors = ERR_NONE;

    // Check format first
    if (sscanf(date, "%2d/%2d/%4d%n", &d, &m, &y, &pos) != 3 || date[pos] != '\0') {
        errors |= ERR_FORMAT;
        return errors;  // can't trust values if format is bad
    }

    // Check month range
    if (m < 1 || m > 12)
        errors |= ERR_MONTH_RANGE;

    // Check day range
    if (d < 1 || d > 31)
        errors |= ERR_DAY_RANGE;
    // check day range specific to each month (except feb)
    if (m <= 7) {
        if(m % 2 == 0 && d == 31)
            errors |= ERR_DAY_RANGE;
    } else if(m % 2 == 1 && d == 31) 
            errors |= ERR_DAY_RANGE;

    // Check year range
    if (y < 1900 || y > 2100)
        errors |= ERR_YEAR_RANGE;

    // Check leap year rule if February 29
    int leap = (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
    if (m == 2 && d > (leap ? 29 : 28))
        errors |= ERR_LEAP_DAY;

    // Check if date is not in the future

    // get current year, month and day
    time_t t = time(NULL);
    struct tm *now = localtime(&t);
    int sys_year = now->tm_year + 1900, sys_month = now->tm_mon + 1, sys_day = now->tm_mday;

    if (y * 10000 + m * 100 + d > sys_year * 10000 + sys_month * 100 + sys_day ) 
        errors |= ERR_FUTURE_DATE;

    return errors;  // can be 0 (no error) or combination
}

// returns a random integer in interval [low, up]
int randomIn(int low, int up) {
    if(up < low) {
        int tmp = up;
        up = low;
        low = tmp;
    }
    return low + rand() % (up - low + 1);
}

// creates a random weather measurements file named fname of size num_recs
FILE *createRandomFile(const char *fname, int num_recs) {

    // create a file in wb+ mode
    FILE *fp = fopen(fname, "wb+");
    // check that file was successfully open
    if(!fp) {
        printf("Error creating file!\n");
        return NULL;
    }
    t_rec1 buffer; // buffer for file input

    // open wilayas.txt
    FILE *fp_wilayas = fopen("wilayas.txt", "r"); // open wilayas.txt file
    if(!fp_wilayas) {
        printf("Error opening wilayas.txt!\n");
        fclose(fp);
        return NULL;
    }
    char line_wilaya[255]; // buffer to store each line from wilayas.txt



    // generate records randomly
    for(int i = 1; i < num_recs+1; i++) {
        // 1. generate wilaya randomly
        int wilaya = randomIn(1, 58);

        // search for the name of the generated wilaya in wilayas.txt
        while(fgets(line_wilaya, sizeof(line_wilaya), fp_wilayas)) {
            
            int wilaya_num; // number of wilaya read from wilayas.txt
            char wilaya_name[25]; // name of wilaya read from wilayas.txt

            // parse line_wilaya to wilaya_num and wilaya_name
            sscanf(line_wilaya, "%2d %20[^\n]", &wilaya_num, wilaya_name);
            
            if(wilaya_num == wilaya) { // target wilaya reached
                strcpy(buffer.wilaya, wilaya_name); // write wilaya name in buffer
                break;
            }
        }
        rewind(fp_wilayas); // rewind to the start of wilayas.txt for the next record
        

        // 2. generate temperature randomly

        // ensure the most frequent temperatures are the generated the most
        int temperature;
        int random_percentage = randomIn(1,100);
        if(random_percentage <= 80) temperature = randomIn(10, 30); // the most likely temperature
            else if (random_percentage <= 98) temperature = randomIn(-15, 45);
                else temperature = randomIn(-90, 60);

        buffer.temp = temperature; // write temperature in buffer.temp

        // 3. generate date randomly and ensure it is valid
        char date[11]; // string to store randomly generated date
        do {
            // get system date year
            time_t t = time(NULL);
            struct tm *now = localtime(&t);
            int system_year = now->tm_year + 1900;
            
            // generate day, month, year (year requires system year)
            int d = randomIn(1, 31), m = randomIn(1, 12), y = randomIn(system_year-2, system_year);

            snprintf(date, sizeof date, "%02d/%02d/%04d", d, m, y); // date = "dd/mm/yyyy"
        } while(isValidDate(date) != ERR_NONE); // regenerate date until it is valid (return value = 0)
        strcpy(buffer.date, date); // write generated date in buffer

        // finally write buffer in weather measurements file
        fwrite(&buffer, sizeof(buffer), 1, fp);

    }
    fclose(fp_wilayas); // close wilayas.txt
    return fp;
}

// creates an empty weather measurements file (returns NULL if file is not found)
FILE *createEmptyFile(const char *fname) {
    return fopen(fname, "wb+");
}

// opens an existing weather measurements file (returns NULL if file is not found)
FILE *openExistingFile(const char *fname) {
    return fopen(fname, "rb+");
}

// inserts a weather measurement record at the end of the file
// must check that buffer buf is valid before function call
void insert(FILE *f, t_rec1 *buf) {
    // insert at the end of the file
    fseek(f, 0, SEEK_END); 
    fwrite(buf, sizeof(t_rec1), 1, f);
    rewind(f);
}

// modifies the temperature of a given wilaya on a given date
int modifyTemp(const char *wilaya, const char *date, int temp, FILE *f) {
    t_rec1 buf;
    // read record by record
    while(fread(&buf, sizeof buf, 1, f) == 1) {
        // check if the current record is the one searched
        if(strcmp(buf.wilaya, wilaya) == 0 && strcmp(buf.date, date) == 0) {
            // modify temperature
            buf.temp = temp;
            // write modified buffer
            fwrite(&buf, sizeof buf, 1, f);
            rewind(f); // rewind before returning
            return 0; // record found and modified (success)
        }
    }
    rewind(f);
    return 1; // record not found the given (wilaya, date) pair doesn't exist in file
}

// deletes all records of a given wilaya
FILE *deleteWilaya(const char *wilaya, FILE *f) {
    // open new file
    FILE *f_new = fopen("new_temporary_deleted_wilayas_file.bin", "wb+");
    // check new file
    if(!f_new) {
        printf("Error creating temporary file with deleted wilaya!\n");
        rewind(f);
        return NULL;
    }
    
    t_rec1 buf; // declare buffer
    // copy all records except ones which have the deleted wilaya
    while(fread(&buf, sizeof buf, 1, f) == 1){
        if(strcmp(buf.wilaya, wilaya) != 0) {
            fwrite(&buf, sizeof buf, 1, f_new);
        }
    } 
    fclose(f);
    rename("new_temporary_deleted_wilayas_file.bin", "filename.bin"); // rename new file to old one
    rewind(f_new);
    return f_new;
}

// prints min, max temperatures of a given wilaya with their corresponding dates
// and prints average temperature of the given wilaya
void wilayaStats(const char *wilaya, FILE *f) {
    // position fp to the beginning
    fseek(f, 0, SEEK_SET);

    // initialize variables
    int wilaya_freq = 0;
    int min = 100, max = -100, avg = 0;
    char wilaya[25], datemin[11], datemax[11];

    t_rec1 buf; // declare file buffer

    while(fread(&buf, sizeof(buf), 1, f) == 1){
        //
        if (strcmp(buf.wilaya, wilaya) == 0) {
            min = (buf.temp < min)? buf.temp : min;
            max = (buf.temp > max)? buf.temp : max;
            wilaya_freq++;
            avg += buf.temp;
        }
    }
    if (wilaya_freq < 1) {
        printf("There are no records of the wilaya '&s'!\n", wilaya);
        return;
    }
    avg = avg / wilaya_freq; // compute average temperature
    // finally print min, max, avg and the corresponding dates
    printf("The minimum temperature of wilaya %s is %d on %s\n", wilaya, min, datemin);
    printf("The maximum temperature of wilaya %s is %d on %s\n", wilaya, max, datemax);
    printf("The average temperature of wilaya %s is %d\n", wilaya, avg);
}


// prints weather mesurements file
void printFile(FILE *f) {
    // declare buffer
    t_rec1 buf;
    // read record by record
    int count = 1;
    while(fread(&buf, sizeof buf, 1, f) == 1) {
        // print record
        printf("%d. < wilaya = %s , date = %s , temperature = %d >\n", count++, buf.wilaya, buf.date, buf.temp);
    }
    rewind(f);
}


