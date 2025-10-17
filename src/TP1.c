#include "../lib/TP1.h"

// ---- Functions Implementation ----
// 1. weather mesurements

// 1.1 helping functions

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

    if (!(y <= sys_year && m <= sys_month && d <= sys_day))
        errors |= ERR_FUTURE_DATE;

    return errors;  // can be 0 (no error) or combination
}

// returns a random integer in interval [low, up]
int random(int low, int up) {
    return ((rand() % up-low+1) + up);
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
        return NULL;
    }
    char line_wilaya[255]; // buffer to store each line from wilayas.txt

    // randomize seed to ensure any numbers generated randomly in this program are actually random
    srand(time(NULL));

    // generate records randomly
    for(int i = 1; num_recs+1; i++) {
        // 1. generate wilaya randomly
        int wilaya = random(1, 58);

        // search for the name of the generated wilaya in wilayas.txt and store it in buffer.wilaya
        while(!feof(fp_wilayas)) {
            // save a line from wilayas.txt
            // containing wilaya_num wilaya_name in the string line_wilaya
            fgets(line_wilaya, sizeof(line_wilaya), fp_wilayas);
            
            int wilaya_num; // number of wilaya read from wilayas.txt
            char *wilaya_name; // name of wilaya read from wilayas.txt

            // parse line_wilaya to wilaya_num and wilaya_name
            sscanf(line_wilaya, "%2d %s", &wilaya_num, wilaya_name);
            
            if(wilaya_num == wilaya) { // current wilaya read is the one randomly generated
                // store wilaya name in buffer.wilaya
                buffer.wilaya = wilaya_name;
                break;
            }
        }
        rewind(fp_wilayas); // rewind to the start of wilayas.txt for the next record

        // 2. generate temperature randomly

        // ensure the most frequent temperatures are the generated the most
        for(int j=0; 100; j++){
            if(j < 79) buffer.temp = random(10, 30); // the most likely temperature is between 10 and 30 degrees
            else if (j < 98) buffer.temp = random(-15, 45);
            else buffer.temp = random(-90, 60);
        }

        // 3. generate date randomly and ensure it is valid
        char date[11]; // string to store randomly generated date
        do {
            // get system date year
            time_t t = time(NULL);
            struct tm *now = localtime(&t);
            int system_year = now->tm_year + 1900;
            
            // generate day, month, year (year requires system year)
            int d = random(1, 31), m = random(1, 12), y = random(system_year-2, system_year);

            snprintf(date, sizeof date, "%02d/%02d/%04d",&d, &m, &y); // date = "dd/mm/yyyy"
        } while(!isValidDate(date)); // regenerate date until it is valid (return value = 0)
        buffer.date = date; // write randomly generated date in buffer.date

        // finally write buffer in wether measurements file
        fwrite(&buffer, sizeof(buffer), 1, fp);

    }
    /* remember: FCLOSE each open file*/
    fclose(fp_wilayas); // close wilayas.txt
    fclose(fp); // close randomly generated file
    return fp;
}






