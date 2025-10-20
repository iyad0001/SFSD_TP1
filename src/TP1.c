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

// gets wilaya name given its number
char *getWilayaName(int wilaya_num) {
    static char wilaya_name[25]; // returned string
    wilaya_name[0] = '\0';

    // open wilayas.txt
    FILE *fp_wilayas = fopen("wilayas.txt", "r"); // open wilayas.txt file
    if(!fp_wilayas) {
        printf("Error opening wilayas.txt!\n");
        return wilaya_name;
    }

    char line_wilaya[255]; // buffer to store each line from wilayas.txt

    // search for the name of the given wilaya in wilayas.txt
    while(fgets(line_wilaya, sizeof(line_wilaya), fp_wilayas)) {
        
        int num; // number of wilaya at current line
        // parse line_wilaya to wilaya_num and wilaya_name
        sscanf(line_wilaya, "%2d %20[^\n]", &num, wilaya_name);
        
        if(num == wilaya_num) { 
            // target wilaya reached
            break;
        }
    }
    rewind(fp_wilayas);
    fclose(fp_wilayas);
    return wilaya_name;
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
// assumes file is open
void insert(FILE *f, t_rec1 buf) {
    // insert at the end of the file
    fseek(f, 0, SEEK_END); 
    fwrite(&buf, sizeof(t_rec1), 1, f);
    rewind(f);
}

// modifies the temperature of a given wilaya on a given date
// returns 0 if record is found and modified (success), 1 if record wasn't found
// assumes file is open
int modifyTemp(const char *wilaya, const char *date, float temp, FILE *f) {
    t_rec1 buf;
    // read record by record
    while(fread(&buf, sizeof buf, 1, f) == 1) {
        // check if the current record is the one searched
        if(strcmp(buf.wilaya, wilaya) == 0 && strcmp(buf.date, date) == 0) {
            // go back by just one record
            fseek(f, -((long)sizeof(buf)), SEEK_CUR);
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
// assumes file is open
FILE *deleteWilaya(const char *wilaya, FILE *f, char *filename) {
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
    rename("new_temporary_deleted_wilayas_file.bin", filename); // rename new file to old one
    rewind(f_new);
    return f_new;
}

// prints min, max temperatures of a given wilaya with their corresponding dates and avg temp
// assumes file is open
void wilayaStats(const char *wilaya, FILE *f) {
    // position fp to the beginning
    fseek(f, 0, SEEK_SET);

    // initialize variables
    int wilaya_freq = 0;
    float min = 100.0f, max = -100.0f, avg = 0.0f;
    char datemin[11], datemax[11];

    t_rec1 buf; // declare file buffer

    while(fread(&buf, sizeof(buf), 1, f) == 1){
        //
        if (strcmp(buf.wilaya, wilaya) == 0) {

            if(buf.temp < min) {
                strcpy(datemin, buf.date);
                min = buf.temp;
            }
            if(buf.temp > max) {
                strcpy(datemax, buf.date);
                max = buf.temp;
            }

            wilaya_freq++;
            avg += buf.temp;
        }
    }
    if (wilaya_freq < 1) {
        printf("There are no records of the wilaya %s!\n", wilaya);
        return;
    }
    avg = avg / wilaya_freq; // compute average temperature
    // finally print min, max, avg and the corresponding dates
    printf("The minimum temperature of wilaya %s is %.2f on %s\n", wilaya, min, datemin);
    printf("The maximum temperature of wilaya %s is %.2f on %s\n", wilaya, max, datemax);
    printf("The average temperature of wilaya %s is %.2f\n", wilaya, avg);
}

// prints weather mesurements file
// assumes file is open
void printFile(FILE *f) {
    printf("Printing file...\n");
    // declare buffer
    t_rec1 buf;
    // read record by record
    int count = 1;
    while(fread(&buf, sizeof buf, 1, f) == 1) {
        // print record
        printf("%d. < wilaya = %s , date = %s , temperature = %.2f >\n", count++, buf.wilaya, buf.date, buf.temp);
    }
    printf("\n");
    rewind(f);
}

// encodes a given file using a key of n bytes
// assumes file is open
FILE *encode(FILE *f, char *fname_encoded, const char *key, int n) {
    // create encoded file (wb+ mode)
    FILE *f_encoded = fopen(fname_encoded, "wb+");
    if(!f_encoded) {
        printf("Error creating file!\n");
        return NULL;
    }

    // declare buffer
    unsigned char buf[n]; // array of n bytes
    int remaining_bytes; // integer containing leftover bytes after reading file < n
    while((remaining_bytes = fread(buf, 1, n, f)) == n) {
        // encode read buffer (encode n bytes)
        for (int i = 0; i < n; i++)
        {
            // if buf[i] is non-printable leave it as it
            if (buf[i] > 32) {
                buf[i] += key[i];
                while(!(buf[i] >= 32 && buf[i] <= 126)) {
                    buf[i] += 32-127;
                }
            }
        }
        
        // write encoded buffer
        fwrite(buf, 1, n, f_encoded);
    }
    // encode remaining bytes
    for (int i = 0; i < remaining_bytes; i++)
    {
        // if buf[i] is non-printable leave it as it
        if (buf[i] > 32) {
            buf[i] += key[i];
            while(!(buf[i] >= 32 && buf[i] <= 126)) {
                buf[i] += 32-127;
            }
        }
    }
    // write last buffer
    fwrite(buf, 1, remaining_bytes, f_encoded);
    rewind(f);
    return f_encoded; // finally return a pointer to the encoded file
}
 
// decodes a given file using a key of n bytes
// assumes file is open
FILE *decode(FILE *f, char *fname_decoded, const char *key, int n) {
    // create decoded file (wb+ mode)
    FILE *f_decoded = fopen(fname_decoded, "wb+");
    if(!f_decoded) {
        printf("Error creating file!\n");
        return NULL;
    }

    // declare buffer
    unsigned char buf[n]; // array of n bytes
    int remaining_bytes; // integer containing leftover bytes after reading file < n
    while((remaining_bytes = fread(buf, 1, n, f)) == n) {
        // decode read buffer (decode n bytes)
        for (int i = 0; i < n; i++)
        {
            // if buf[i] is non-printable leave it as it
            if (buf[i] > 32) {
                buf[i] += key[i];
                while(!(buf[i] >= 32 && buf[i] <= 126)) {
                    buf[i] += -32+127;
                }
            }
        }
        
        // write decoded buffer
        fwrite(buf, 1, n, f_decoded);
    }
    // decode remaining bytes
    for (int i = 0; i < remaining_bytes; i++)
    {
        // if buf[i] is non-printable leave it as it
        if (buf[i] > 32) {
            buf[i] += key[i];
            while(!(buf[i] >= 32 && buf[i] <= 126)) {
                buf[i] += -32+127;
            }
        }
    }
    // write last buffer
    fwrite(buf, 1, remaining_bytes, f_decoded);
    rewind(f);
    return f_decoded; // finally return a file pointer to the decoded file
}
 
// main menu function
void mainMenu() {
    int choice;
    do {
        // print main menu
        printf("\n<===============  Main Menu  ===============>\n");
        printf("\t1. Temperature measurements\n");
        printf("\t2. CCP transfers management\n");
        printf("\t3. Exit\n");

        // read choice
        printf("Enter choice: ");
        scanf("%d", &choice);
        switch(choice) {
            case 1: menu1(); break;
            // case 2: menu2(); break;
            case 3: printf("Exiting...\n"); break;
            default: printf("Invalid choice!\n");
        }

    } while(choice != 3);
}

// prints temperature measurements menu
void menu1() {
    int choice;
    // declare variables for this menu and its children menus
    char filename[50], filename_encoded[50], filename_decoded[50];
    FILE *f, *f_encoded, *f_decoded;
    do
    {
        // print temperature measurements menu
        printf("\n<===============  Temperature Measurements  ===============>\n");
        printf("\t1. Create/Open file\n");
        printf("\t2. Print file\n");
        printf("\t3. Functions on measurements\n");
        printf("\t4. Encode/Decode measurements file\n");
        printf("\t5. Back to main menu\n");

        // read choice
        printf("Enter choice: ");
        scanf("%d", &choice);
        switch(choice) {
            case 1: menu1_1(filename, &f); break;
            case 2: printFile(f); break;
            case 3: menu1_3(&f); break;
            case 4: menu1_4(&f, filename_encoded, filename_decoded, &f_encoded, &f_decoded); break;
            case 5: printf("Returning to main menu...\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 5);
}

// Create/open file menu
void menu1_1(char *filename, FILE **f) {
    int choice;
    do
    {
        // print temperature measurements menu
        printf("\n<===============  Create/Open file  ===============>\n");
        printf("\t1. Create empty measurements file\n");
        printf("\t2. Create file randomly\n");
        printf("\t3. Open existing file\n");
        printf("\t4. Back to main menu\n");

        // read choice
        printf("Enter choice: ");
        scanf("%d", &choice);
        switch(choice) {
            case 1: // create empty file

                /* [] validate filename with function + loop */

                printf("Enter the name of the file you want to create: "); 
                scanf("%49s", filename);
                // create empty file
                *f = createEmptyFile(filename);
                break;

            case 2: // create random file
                // read filename
                printf("Enter the name of the file you want to create: "); 
                scanf("%49s", filename);

                // read the number of records
                printf("Enter the number of measurements: "); 
                int num_recs;
                scanf("%d", &num_recs);

                // create random file
                *f = createRandomFile(filename, num_recs);
                break;

            case 3: // open existing file
                // read filename
                printf("Enter the name of the file you want to open (file path): "); 
                scanf("%49s", filename);
                // open file
                *f = openExistingFile(filename);
                break;

            case 4: printf("Returning to previous menu...\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 4);
}

// Functions on file (insert, modify, delete, stats)
void menu1_3(FILE **f) {
    int choice;
    do {
        // print main menu
        printf("\n<===============  Function on measurements file  ===============>\n");
        printf("\t1. Insert a measurement\n");
        printf("\t2. Modify a measurement temperature\n");
        printf("\t3. Display measurement stats of a wilaya\n");
        printf("\t4. Quit\n");

        // read choice
        printf("Enter choice: ");
        scanf("%d", &choice);
        int wilaya_num;
        char date[11];
        float temp;
        t_rec1 buffer;

        int measurement_found;
        int isMeasurementValid; 
        switch(choice) {
            case 1: // insert a measurement
            // first: read record to insert
            
            do {
                // read wilaya
                printf("Enter wilaya number (1-58): ");
                scanf("%d", &wilaya_num);
                // read date
                printf("Enter a valid date: ");
                scanf("%10s", date);
                // read temperature
                printf("Enter temperature: ");
                scanf("%f", &temp);
                printf("\n");
            } while(!((wilaya_num >= 1 && wilaya_num <= 58) && (isValidDate(date) == ERR_NONE) && (temp >= -90 && temp <= 60)));

            // fill record
            strcpy(buffer.wilaya, getWilayaName(wilaya_num));
            strcpy(buffer.date, date);
            buffer.temp = temp;

            // function call
            insert(*f, buffer);
            break;

            case 2: // modify measurement temperature
                do {
                    // read wilaya
                    printf("Enter wilaya number (1-58): ");
                    scanf("%d", &wilaya_num);
                    // read date
                    printf("Enter a valid date: ");
                    scanf("%10s", date);
                    // read temperature
                    printf("Enter temperature: ");
                    scanf("%f", &temp);
                    printf("\n");

                    isMeasurementValid = ((wilaya_num >= 1 && wilaya_num <= 58) && (isValidDate(date) == ERR_NONE) && (temp >= -90 && temp <= 60));
                    if (!isMeasurementValid)
                        continue;
                    
                    measurement_found = modifyTemp(getWilayaName(wilaya_num), date, temp, *f);
                    if(measurement_found == 1) printf("Measurement with wilaya %d and date %s NOT found!\n", wilaya_num, date);
                    else printf("Measurement with wilaya %d and date %s was set to %f\n", wilaya_num, date, temp);
                } while(!isMeasurementValid);
                break;
                
            case 3: // display wilaya stats
                // read wilaya_num

                do
                {
                    printf("Enter wilaya number (1-58): ");
                    scanf("%d", &wilaya_num);                    
                } while (wilaya_num >= 1 && wilaya_num <= 58);

                wilayaStats(getWilayaName(wilaya_num), *f);
                break;

            case 4: printf("Returning to previous menu...\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while(choice != 4);
}

// Encode/Decode file
void menu1_4(FILE **f, char *filename_encoded, char *filename_decoded, FILE **f_encoded, FILE **f_decoded) {
    int choice;
    do {
        // print menu
        printf("\n<===============  Encode/Decode file  ===============>\n");
        printf("\t1. Encode file\n");
        printf("\t2. Decode file\n");
        printf("\t3. Quit\n");

        // read choice
        printf("Enter choice: ");
        scanf("%d", &choice);

        // declare key and its lenght n
        int n = 10;
        char key[n];
        int haveRead = 0;
        char read_again;

        switch(choice) {
            case 1: 
                // read output file name
                printf("Enter the name of the output file (encoded file): "); 
                scanf("%49s", filename_encoded);

                // read n and key
                if(haveRead != 0) {
                    printf("keep last read (key,n) pair or read again?\n");
                    printf("choice (y/n): ");
                    scanf("%c", &read_again);

                    if(read_again == 'y' || read_again == 'Y') {
                        printf("Enter key lenght 'n': ");
                        scanf("%d", &n);
                        printf("Enter key: ");
                        scanf("%s", key);
                    }
                } else {
                    printf("Enter key lenght 'n': ");
                    scanf("%d", &n);
                    printf("Enter key: ");
                    scanf("%s", key);
                    haveRead = 1;
                }

                *f_encoded = encode(*f, filename_encoded, key, n);
                printf("File successfully encoded\n");
                break;

            case 2: 
                // read output file name
                printf("Enter the name of the output file (decoded file): "); 
                scanf("%49s", *filename_decoded);

                // read n and key
                if(haveRead != 0) {
                    printf("keep last read (key,n) pair or read again?\n");
                    printf("choice (y/n): ");
                    scanf("%c", read_again);

                    if(read_again == 'y' || read_again == 'Y') {
                        printf("Enter key lenght 'n': ");
                        scanf("%d", &n);
                        printf("Enter key: ");
                        scanf("%s", key);
                    }
                } else {
                    printf("Enter key lenght 'n': ");
                    scanf("%d", &n);
                    printf("Enter key: ");
                    scanf("%s", key);
                    haveRead = 1;
                }

                *f_decoded = decode(*f, filename_decoded, key, n);
                printf("File successfully decoded\n");
                break;

            case 3: printf("Returning to previous menu...\n"); break;
            default: printf("Invalid choice!\n");
        }

    } while(choice != 3);
}

