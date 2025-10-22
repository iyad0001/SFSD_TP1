#include "TP1.h"

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
        sscanf(line_wilaya, "%2d %24[^\n]", &num, wilaya_name);
        
        if(num == wilaya_num) { 
            // target wilaya reached
            break;
        }
    }
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
            sscanf(line_wilaya, "%2d %24[^\n]", &wilaya_num, wilaya_name);
            
            if(wilaya_num == wilaya) { // target wilaya reached
                strncpy(buffer.wilaya, wilaya_name, sizeof(buffer.wilaya)-1);
                buffer.wilaya[sizeof(buffer.wilaya)-1] = '\0';
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

        buffer.temp = (float)temperature; // write temperature in buffer.temp

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
        strncpy(buffer.date, date, sizeof(buffer.date)-1);
        buffer.date[sizeof(buffer.date)-1] = '\0'; // write generated date in buffer

        // finally write buffer in weather measurements file
        fwrite(&buffer, sizeof(buffer), 1, fp);

    }
    fclose(fp_wilayas); // close wilayas.txt
    rewind(fp); // place created file at beginning
    return fp;
}

// creates an empty weather measurements file (returns NULL if file is not found)
FILE *createEmptyFile(const char *fname) {
    FILE *fp = fopen(fname, "wb+");
    if(fp) rewind(fp);
    return fp;
}

// opens an existing weather measurements file (returns NULL if file is not found)
FILE *openExistingFile(const char *fname) {
    FILE *fp = fopen(fname, "rb+");
    if(fp) rewind(fp);
    return fp;
}

// inserts a weather measurement record at the end of the file
// must check that buffer buf is valid before function call
// assumes file is open
void insert(FILE *f, t_rec1 buf) {
    if(!f) {
        printf("File is not open for insert.\n");
        return;
    }

    t_rec1 tmp_buf;
    // search for (wilaya, date) pair, insert only if it doesn't already exist
    while(fread(&tmp_buf, sizeof(tmp_buf), 1, f) == 1) {
        if(strcmp(tmp_buf.wilaya, buf.wilaya) == 0 && strcmp(tmp_buf.date, buf.date) == 0) {
            // go back by just one record
            if (fseek(f, -((long)sizeof(tmp_buf)), SEEK_CUR) != 0) {
                perror("fseek failed");
                rewind(f);
                return;
            }
            // modify temperature
            tmp_buf.temp = buf.temp;
            // write modified tmp_buffer
            if (fwrite(&tmp_buf, sizeof(tmp_buf), 1, f) != 1) {
                perror("fwrite failed");
            }
            fflush(f);
            rewind(f); // rewind before returning
            return;
        }
    }

    // insert at the end of the file
    if (fseek(f, 0, SEEK_END) != 0) {
        perror("fseek failed");
        rewind(f);
        return;
    }
    if (fwrite(&buf, sizeof(t_rec1), 1, f) != 1) {
        perror("fwrite failed");
    }
    fflush(f);
    rewind(f);
}

// modifies the temperature of a given wilaya on a given date
// returns 0 if record is found and modified (success), 1 if record wasn't found
// assumes file is open
int modifyTemp(const char *wilaya, const char *date, float temp, FILE *f) {
    if(!f) return 1;
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
    if(!f) return NULL;
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
    if(rename("new_temporary_deleted_wilayas_file.bin", filename) != 0) {
        printf("Error renaming temporary file to %s\n", filename);
        fclose(f_new);
        return NULL;
    }
    rewind(f_new);
    return f_new;
}

// prints min, max temperatures of a given wilaya with their corresponding dates and avg temp
// assumes file is open
void wilayaStats(const char *wilaya, FILE *f) {
    if(!f) {
        printf("File is not open.\n");
        return;
    }
    // position fp to the beginning
    fseek(f, 0, SEEK_SET);

    // initialize variables
    int wilaya_freq = 0;
    float min = 100.0f, max = -100.0f, avg = 0.0f;
    char datemin[11] = "", datemax[11] = "";

    t_rec1 buf; // declare file buffer

    while(fread(&buf, sizeof(buf), 1, f) == 1){
        //
        if (strcmp(buf.wilaya, wilaya) == 0) {

            if(buf.temp < min) {
                strncpy(datemin, buf.date, sizeof(datemin)-1);
                datemin[sizeof(datemin)-1] = '\0';
                min = buf.temp;
            }
            if(buf.temp > max) {
                strncpy(datemax, buf.date, sizeof(datemax)-1);
                datemax[sizeof(datemax)-1] = '\0';
                max = buf.temp;
            }

            wilaya_freq++;
            avg += buf.temp;
        }
    }
    if (wilaya_freq < 1) {
        printf("There are no records of the wilaya %s!\n", wilaya);
        rewind(f);
        return;
    }
    avg = avg / wilaya_freq; // compute average temperature
    // finally print min, max, avg and the corresponding dates
    printf("The minimum temperature of wilaya %s is %.2f on %s\n", wilaya, min, datemin);
    printf("The maximum temperature of wilaya %s is %.2f on %s\n", wilaya, max, datemax);
    printf("The average temperature of wilaya %s is %.2f\n", wilaya, avg);
    rewind(f);
}

// prints weather mesurements file
// assumes file is open
void printFile(FILE *f) {
    if(!f) {
        printf("No file open to print.\n");
        return;
    }
    printf("Printing file...\n");
    // declare buffer
    t_rec1 buf;
    // read record by record
    int count = 1;
    fseek(f, 0, SEEK_SET);
    while(fread(&buf, sizeof buf, 1, f) == 1) {
        // print record
        printf("%d. < wilaya = %s , date = %s , \ttemperature = %.2f >\n", count++, buf.wilaya, buf.date, buf.temp);
    }
    printf("\n");
    rewind(f);
}

// encodes a given file using a key of n bytes
// assumes file is open
FILE *encode(FILE *f, char *fname_encoded, const char *key, int n) {
    if(!f || !fname_encoded || !key || n <= 0) {
        printf("Invalid parameters for encode.\n");
        return NULL;
    }

    // prepare key bytes of length n by repeating provided key as needed
    int keylen = (int)strlen(key);
    if(keylen == 0) {
        printf("Empty key provided.\n");
        return NULL;
    }
    unsigned char *kbytes = malloc((size_t)n);
    if(!kbytes) return NULL;
    for(int i = 0; i < n; i++) kbytes[i] = (unsigned char)key[i % keylen];

    // rewind input file
    rewind(f);

    // create encoded file (wb+ mode)
    FILE *f_encoded = fopen(fname_encoded, "wb+");
    if(!f_encoded) {
        printf("Error creating file!\n");
        free(kbytes);
        return NULL;
    }

    // process in blocks
    unsigned char buf[4096];
    size_t r;
    size_t pos = 0; // position in stream to apply key cyclically
    while((r = fread(buf, 1, sizeof(buf), f)) > 0) {
        for(size_t i = 0; i < r; i++) {
            unsigned char b = buf[i];
            if(b >= 32 && b <= 126) {
                // printable range is 32..126 inclusive => 95 characters
                int v = (int)(b - 32);
                int k = kbytes[(pos % (size_t)n)];
                v = (v + (k & 0xFF)) % 95;
                buf[i] = (unsigned char)(v + 32);
            }
            pos++;
        }
        fwrite(buf, 1, r, f_encoded);
    }

    rewind(f);
    rewind(f_encoded);
    free(kbytes);
    return f_encoded; // finally return a pointer to the encoded file
}
 
// decodes a given file using a key of n bytes
// assumes file is open
FILE *decode(FILE *f, char *fname_decoded, const char *key, int n) {
    if(!f || !fname_decoded || !key || n <= 0) {
        printf("Invalid parameters for decode.\n");
        return NULL;
    }

    int keylen = (int)strlen(key);
    if(keylen == 0) {
        printf("Empty key provided.\n");
        return NULL;
    }
    unsigned char *kbytes = malloc((size_t)n);
    if(!kbytes) return NULL;
    for(int i = 0; i < n; i++) kbytes[i] = (unsigned char)key[i % keylen];

    // rewind input file
    rewind(f);

    // create decoded file (wb+ mode)
    FILE *f_decoded = fopen(fname_decoded, "wb+");
    if(!f_decoded) {
        printf("Error creating file!\n");
        free(kbytes);
        return NULL;
    }

    unsigned char buf[4096];
    size_t r;
    size_t pos = 0;
    while((r = fread(buf, 1, sizeof(buf), f)) > 0) {
        for(size_t i = 0; i < r; i++) {
            unsigned char b = buf[i];
            if(b >= 32 && b <= 126) {
                int v = (int)(b - 32);
                int k = kbytes[(pos % (size_t)n)];
                v = (v - (k & 0xFF)) % 95;
                if(v < 0) v += 95;
                buf[i] = (unsigned char)(v + 32);
            }
            pos++;
        }
        fwrite(buf, 1, r, f_decoded);
    }

    rewind(f);
    rewind(f_decoded);
    free(kbytes);
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
        if(scanf("%d", &choice) != 1) { // validate input
            int c;
            while((c = getchar()) != EOF && c != '\n');
            choice = -1;
        }
        switch(choice) {
            case 1: menu1(); break;
            case 2: menu2(); break;
            case 3: printf("Exiting...\n"); break;
            default: printf("Invalid choice!\n");
        }

    } while(choice != 3);
}

// prints temperature measurements menu
void menu1() {
    int choice;
    // declare variables for this menu and its children menus
    char filename[50] = "", filename_encoded[50] = "", filename_decoded[50] = "";
    FILE *f = NULL, *f_encoded = NULL, *f_decoded = NULL;
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
        if(scanf("%d", &choice) != 1) {
            int c;
            while((c = getchar()) != EOF && c != '\n');
            choice = -1;
        }
        switch(choice) {
            case 1: menu1_1(filename, &f); break;
            case 2:
                printFile(f);
                break;
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
        if(scanf("%d", &choice) != 1) {
            int c;
            while((c = getchar()) != EOF && c != '\n');
            choice = -1;
        }
        switch(choice) {
            case 1: // create empty file

                /* [] validate filename with function + loop */

                printf("Enter the name of the file you want to create: "); 
                scanf("%49s", filename);
                // create empty file
                *f = createEmptyFile(filename);
                if(!*f) printf("Failed to create file %s\n", filename);
                break;

            case 2: // create random file
                // read filename
                printf("Enter the name of the file you want to create: "); 
                scanf("%49s", filename);

                // read the number of records
                printf("Enter the number of measurements: "); 
                {
                    int num_recs = 0;
                    if(scanf("%d", &num_recs) != 1) num_recs = 0;
                    // create random file
                    *f = createRandomFile(filename, num_recs);
                    if(!*f) printf("Failed to create random file %s\n", filename);
                }
                break;

            case 3: // open existing file
                // read filename
                printf("Enter the name of the file you want to open (file path): "); 
                scanf("%49s", filename);
                // open file
                *f = openExistingFile(filename);
                if(!*f) printf("Failed to open file %s\n", filename);
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
        printf("\n<===============  Functions on measurements file  ===============>\n");
        printf("\t1. Insert a measurement\n");
        printf("\t2. Modify a measurement temperature\n");
        printf("\t3. Display measurement stats of a wilaya\n");
        printf("\t4. Quit\n");

        // read choice
        printf("Enter choice: ");
        if(scanf("%d", &choice) != 1) {
            int c;
            while((c = getchar()) != EOF && c != '\n');
            choice = -1;
        }
        int wilaya_num;
        char date[11];
        float temp;
        t_rec1 buffer;

        int measurement_found;
        int isMeasurementValid; 
        switch(choice) {
            case 1: // insert a measurement
                if(!*f) { printf("No file open. Create or open a file first.\n"); break; }
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
                strncpy(buffer.wilaya, getWilayaName(wilaya_num), sizeof(buffer.wilaya)-1);
                buffer.wilaya[sizeof(buffer.wilaya)-1] = '\0';
                strncpy(buffer.date, date, sizeof(buffer.date)-1);
                buffer.date[sizeof(buffer.date)-1] = '\0';
                buffer.temp = temp;

                // function call
                insert(*f, buffer);
                break;

            case 2: // modify measurement temperature
                if(!*f) { printf("No file open. Create or open a file first.\n"); break; }
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
                    if (!isMeasurementValid) {
                        printf("Invalid input. Try again.\n");
                        continue;
                    }
                    
                    measurement_found = modifyTemp(getWilayaName(wilaya_num), date, temp, *f);
                    if(measurement_found == 1) printf("Measurement with wilaya %d and date %s NOT found!\n", wilaya_num, date);
                    else printf("Measurement with wilaya %d and date %s was set to %f\n", wilaya_num, date, temp);
                } while(!isMeasurementValid);
                break;
                
            case 3: // display wilaya stats
                if(!*f) { printf("No file open. Create or open a file first.\n"); break; }
                do
                {
                    printf("Enter wilaya number (1-58): ");
                    scanf("%d", &wilaya_num);                    
                } while (!(wilaya_num >= 1 && wilaya_num <= 58));

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
        if(scanf("%d", &choice) != 1) {
            int c;
            while((c = getchar()) != EOF && c != '\n');
            choice = -1;
        }

        // declare key buffer and variables
        int n = 10;
        char keybuf[256] = {0};
        int haveRead = 0;
        char read_again = 'n';

        switch(choice) {
            case 1: {
                if(!*f) { printf("No file open. Create or open a file first.\n"); break; }
                // read output file name
                printf("Enter the name of the output file (encoded file): ");
                scanf("%49s", filename_encoded);

                // read n and key
                printf("Enter key length 'n' (positive integer): ");
                if(scanf("%d", &n) != 1 || n <= 0) n = 10;
                printf("Enter key: ");
                scanf("%255s", keybuf);
                haveRead = 1;

                // close previous encoded handle if any
                if(*f_encoded) { fclose(*f_encoded); *f_encoded = NULL; }

                *f_encoded = encode(*f, filename_encoded, keybuf, n);
                if(*f_encoded) printf("File successfully encoded -> %s\n", filename_encoded);
                else printf("Encoding failed.\n");
                break;
            }

            case 2: {
                // choose input encoded file: prefer previously produced encoded file if available
                FILE *input = NULL;
                char encoded_in[260] = "";
                if(*f_encoded) {
                    input = *f_encoded;
                } else {
                    // ask for encoded input filename
                    printf("Enter the name of the encoded input file (to decode): ");
                    scanf("%259s", encoded_in);
                    input = openExistingFile(encoded_in);
                    if(!input) {
                        printf("Failed to open encoded file %s\n", encoded_in);
                        break;
                    }
                }

                // read output file name for decoded output
                printf("Enter the name of the output file (decoded file): ");
                scanf("%49s", filename_decoded);

                // read key length and key (reuse last if available)
                if(haveRead) {
                    printf("Keep last (key,n) pair or read again? (k/r): ");
                    scanf(" %c", &read_again);
                } else {
                    read_again = 'r';
                }
                if(read_again == 'r' || read_again == 'R') {
                    printf("Enter key length 'n' (positive integer): ");
                    if(scanf("%d", &n) != 1 || n <= 0) n = 10;
                    printf("Enter key: ");
                    scanf("%255s", keybuf);
                    haveRead = 1;
                }

                // perform decode
                if(*f_decoded) { fclose(*f_decoded); *f_decoded = NULL; }
                *f_decoded = decode(input, filename_decoded, keybuf, n);
                if(*f_decoded) printf("File successfully decoded -> %s\n", filename_decoded);
                else printf("Decoding failed.\n");

                // if we opened a temporary input file (encoded_in), close it; if it was *f_encoded, leave it open
                if(!*f_encoded && input) {
                    fclose(input);
                }
                break;
            }

            case 3:
                printf("Returning to previous menu...\n");
                break;

            default:
                printf("Invalid choice!\n");
        }

    } while(choice != 3);
}


/*  MENU 2 Functions : */
//----------------------//

FILE* create_random_file1(char *file_name_1, int number_of_accounts , int min_ccp_number ) {
    if (!file_name_1) return NULL;
    FILE *f = fopen(file_name_1, "wb+");
    if (!f) return NULL;

    CCP temp;

    for (int i = 0; i < number_of_accounts; i++) {
        temp.ccp_number = min_ccp_number + i;
        temp.balance = randomIn(1000, 100000000);
        fwrite(&temp, sizeof(CCP), 1, f);
    }
    rewind(f);
    return f;
}

int search_ccp(FILE *file1, int ccp_number) {
    if (file1) {
        CCP temp;
        int found = 0;
        while (fread(&temp, sizeof(CCP), 1, file1) == 1) {
            if (temp.ccp_number == ccp_number) {
                found = 1;
                break;
            }
        }
        rewind(file1);
        return found;
    } else
        return 0;
}

int search_transfer(FILE *file2, int ccp_number) {
    if (file2) {
        CCP temp;
        int found = 0;

        while (fscanf(file2, "%d %d", &temp.ccp_number, &temp.balance) == 2) {
            if (temp.ccp_number == ccp_number) {
                found = 1;
                break;
            }
        }
        rewind(file2);
        return found;
    } else {
        return 0;
    }
}

FILE* create_random_file2(char *file_name, int number_of_transfers, int min_ccp_number, int number_of_accounts) {
    if (!file_name) return NULL;
    FILE* f = fopen(file_name, "w+");
    if (!f) return NULL;

    int max_ccp = min_ccp_number + number_of_accounts - 1;
    int transfers = number_of_transfers;

    for (int i = 0; i < transfers; i++) {
        int ccp = randomIn(min_ccp_number, max_ccp);
        int amount = randomIn(10000, 1000000);
        fprintf(f, "%d %d\n", ccp, amount);
    }
    fflush(f);
    rewind(f);
    return f;
}

void insert_account(FILE *file1, int ccp_number, int balance) {
    if (!file1) return;
    if (!search_ccp(file1, ccp_number)) {
        CCP temp;
        temp.ccp_number = ccp_number;
        temp.balance = balance;
        fseek(file1, 0, SEEK_END);
        fwrite(&temp, sizeof(CCP), 1, file1);
        fflush(file1);
        rewind(file1);
    }
}

void insert_transfer(FILE *file1, FILE *file2, int ccp_number, int balance) {
    if (!file2 || !file1) return;
    if (!search_transfer(file2, ccp_number)) {
        // only insert transfer if the account exists in file1
        if (search_ccp(file1, ccp_number)) {
            fprintf(file2, "%d %d\n", ccp_number, balance);
            fflush(file2);
            rewind(file2);
        }
    }
}

void update (FILE* file1, FILE* file2) {
    if (!file1 || !file2) return;

    rewind(file2);
    int t_ccp, t_amount;
    while (fscanf(file2, "%d %d", &t_ccp, &t_amount) == 2) {
        // search in file1
        CCP acc;
        int found = 0;
        rewind(file1);
        while (fread(&acc, sizeof(CCP), 1, file1) == 1) {
            if (acc.ccp_number == t_ccp) {
                // update balance
                acc.balance += t_amount;
                // move back one record and overwrite
                fseek(file1, -((long)sizeof(CCP)), SEEK_CUR);
                fwrite(&acc, sizeof(CCP), 1, file1);
                fflush(file1);
                found = 1;
                break;
            }
        }
        // if not found, you may decide to insert a new account — currently we ignore missing accounts
    }
    rewind(file1);
    rewind(file2);
}

int printfile_CCP (FILE *file1) {
    if (file1) {
        CCP temp;
        rewind(file1);
        while (fread(&temp, sizeof(CCP), 1, file1) == 1) {
            printf("====================\n");
            printf("CCP account number : %d || CCP account balance :  %d\n", temp.ccp_number, temp.balance);
        }
        rewind(file1);
        return 1;
    } else
        return 0;
}

int printfile_transfers (FILE *file2) {
    if (file2) {
        CCP temp;
        rewind(file2);
        while (fscanf(file2,"%d %d",&temp.ccp_number,&temp.balance) == 2) {
            printf("====================\n");
            printf("CCP account number : %d || Transfer amount :  %d\n", temp.ccp_number, temp.balance);
        }
        rewind(file2);
        return 1;
    } else
        return 0;
}

void menu2 () {
    FILE* file1 = NULL;
    FILE* file2 = NULL;
    FILE* f_encoded = NULL;
    FILE* f_decoded = NULL;
    char file1name[260] = {0}, file2name[260] = {0};
    char file1name_encoded[260] = {0}, file1name_decoded[260] = {0};
    char file2name_encoded[260] = {0}, file2name_decoded[260] = {0};
    int choice = -1;

    do {
        printf("\n  <===============  CCP balance management  ===============>\n");
        printf("  |                 1. Create/Open file                    |\n");
        printf("  |                 2. Print file                          |\n");
        printf("  |                 3. Insert                              |\n");
        printf("  |                 4. Update                              |\n");
        printf("  |                 5. Encode/Decode file                  |\n");
        printf("  |                 6. Back to main menu                   |\n");
        printf("  ==========================================================\n");

        printf ("\nEnter your choice:\n -->  ");
        if (scanf("%d", &choice) != 1) {
            int c; while ((c = getchar()) != EOF && c != '\n');
            choice = -1;
        }

        switch (choice) {
            case 1: menu2_1(&file1, &file2); break;
            case 2: menu2_2(&file1, &file2); break;
            case 3: menu2_3(&file1, &file2); break;
            case 4: update(file1, file2); break;
            case 5: {
                // ask which of the two files to encode/decode
                int which = 0;
                printf("Encode/Decode which file? 1=accounts (binary)  2=transfers (text)\n --> ");
                if (scanf("%d", &which) != 1) {
                    int c; while ((c = getchar()) != EOF && c != '\n');
                    break;
                }
                if (which == 1) {
                    menu1_4(&file1, file1name_encoded, file1name_decoded, &f_encoded, &f_decoded);
                } else if (which == 2) {
                    menu1_4(&file2, file2name_encoded, file2name_decoded, &f_encoded, &f_decoded);
                } else {
                    printf("Invalid selection.\n");
                }
                break;
            }
            case 6: printf("Returning to main menu...\n"); break;
            default: printf("Invalid choice!\n"); break;
        }
        // loop continues until user chooses 6
    } while (choice != 6);

    // close any files left open by this menu
    if (file1) { fclose(file1); file1 = NULL; }
    if (file2) { fclose(file2); file2 = NULL; }
    if (f_encoded) { fclose(f_encoded); f_encoded = NULL; }
    if (f_decoded) { fclose(f_decoded); f_decoded = NULL; }
}

void menu2_3 (FILE** file1, FILE** file2) {
    int choice = -1, ccp_number=0, ccp_balance=0;

    while (choice != 3) {
        printf ("\n===========\n");
        printf ("1. Insert in CCP accounts file\n");
        printf ("2. Insert in transfers file\n");
        printf ("3. Quit\n");
        printf ("===============\n");
        printf ("\nEnter your choice:\n -->  ");
        if (scanf("%d", &choice) != 1) {
            int c; while ((c = getchar()) != EOF && c != '\n');
            choice = -1;
        }
        switch (choice) {
            case 1:
                if (!*file1) { printf("No CCP accounts file open. Use Create/Open file first.\n"); break; }
                printf ("Enter CCP account number: \n--> ");
                scanf ("%d", &ccp_number);
                printf ("Enter CCP account balance: \n--> ");
                scanf ("%d", &ccp_balance);
                insert_account(*file1, ccp_number, ccp_balance);
                break;
            case 2:
                if (!*file2) { printf("No transfers file open. Use Create/Open file first.\n"); break; }
                printf ("Enter CCP account number: \n--> ");
                scanf ("%d", &ccp_number);
                printf ("Enter transfer amount: \n--> ");
                scanf ("%d", &ccp_balance);
                insert_transfer(*file1, *file2, ccp_number, ccp_balance);
                break;
            case 3:
                printf("Returning...\n");
                break;
            default:
                printf("Invalid choice!\n");
                break;
        }
    }
}

void menu2_1(FILE **file1, FILE **file2) {
    int choice=-1;
    char filename1[260] = {0};
    char filename2[260] = {0};

    int number_of_accounts = 0; int min_ccp_number = 0; int number_of_transfers = 0;
    do {
        printf("\n ==>  Create/Open_file : \n");
        printf("   1. Create Random files (accounts + transfers)\n");
        printf("   2. Create Empty files\n");
        printf("   3. Open existing files\n");
        printf("   4. Quit\n");
        printf("  ======\n");

        printf ("\nEnter your choice:\n -->  ");
        if (scanf("%d", &choice) != 1) {
            int c; while ((c = getchar()) != EOF && c != '\n');
            choice = -1;
        }

        switch (choice) {
             case 1 :
                printf("Enter the number of accounts : \n --> ");
                scanf("%d", &number_of_accounts);
                printf("Enter the minimum CCP number :\n --> ");
                scanf("%d", &min_ccp_number);
                printf("Enter the number of transfers :\n --> ");
                scanf("%d", &number_of_transfers);
                printf("Enter the file name of CCP accounts : \n --> ");
                scanf("%259s", filename1);
                printf("Enter the file name of CCP transfers : \n --> ");
                scanf("%259s", filename2);

                if (*file1) { fclose(*file1); *file1 = NULL; }
                if (*file2) { fclose(*file2); *file2 = NULL; }

                *file1 = create_random_file1(filename1, number_of_accounts, min_ccp_number);
                *file2 = create_random_file2(filename2, number_of_transfers, min_ccp_number, number_of_accounts);
                if (!*file1) printf("Failed to create CCP accounts file %s\n", filename1);
                if (!*file2) printf("Failed to create transfers file %s\n", filename2);
                break;

            case 2 :
                printf("Enter the file name of CCP accounts : \n --> ");
                scanf("%259s", filename1);
                if (*file1) { fclose(*file1); *file1 = NULL; }
                *file1 = fopen(filename1, "wb+");
                if (!*file1) printf("Failed to create/open %s\n", filename1);

                printf("Enter the file name of CCP transfers : \n --> ");
                scanf("%259s", filename2);
                if (*file2) { fclose(*file2); *file2 = NULL; }
                *file2 = fopen(filename2, "w+");
                if (!*file2) printf("Failed to create/open %s\n", filename2);
                break;

            case 3 :
                printf("Enter the file name of CCP accounts : \n --> ");
                scanf("%259s", filename1);
                if (*file1) { fclose(*file1); *file1 = NULL; }
                *file1 = fopen(filename1, "r+b");
                if (!*file1) {
                    printf("File %s could not be opened\n", filename1);
                    *file1 = NULL;
                } else {
                    printf("Enter the file name of CCP transfers\n --> ");
                    scanf("%259s", filename2);
                    if (*file2) { fclose(*file2); *file2 = NULL; }
                    *file2 = fopen(filename2, "r+");
                    if (!*file2) {
                        printf("File %s could not be opened\n", filename2);
                        *file2 = NULL;
                    }
                }
                break;

            case 4 :
                printf("Returning...\n");
                break;

            default: printf("Invalid choice!\n"); break;
        }

    } while (choice != 4);
}

void menu2_2(FILE **file1, FILE **file2) {
    int choice=-1;
    do {
        printf("\n ==>  Print file : \n");
        printf("   1. CCP accounts file       \n");
        printf("   2. Transfers file         \n");
        printf("   3. Quit      \n");
        printf("Enter your choice:\n -->  ");
        if (scanf("%d", &choice) != 1) {
            int c; while ((c = getchar()) != EOF && c != '\n');
            choice = -1;
        }
        switch (choice) {
            case 1 :
                if (!*file1) { printf("No CCP accounts file open.\n"); break; }
                printf ("========== CCP accounts file : ===============\n");
                printfile_CCP (*file1);
                break;
            case 2 :
                if (!*file2) { printf("No transfers file open.\n"); break; }
                printf ("========== CCP transfers file : ===============\n");
                printfile_transfers (*file2);
                break;
            case 3:
                printf("Returning...\n");
                break;
            default: printf("Invalid choice!\n"); break;
        }
    } while (choice != 3);
}



