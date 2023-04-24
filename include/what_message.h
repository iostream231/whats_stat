# include <ctype.h> 
# include <string.h>
# include <regex.h>


# include "./what_types.h"
# include "./what_storage.h"


// Parses Time And puts results in `msg`. Returns a pointer to the string after time.
static const unsigned int whatsapp_year_prefix = 2000;
static const char *const whatsapp_special_utf_char = " ";
char * parse_time(char * message, struct message * msg) {
    int component, i;
    char num[3];
    num[0] = num[1] = num[2] = '\0'; // Init vars 

    // Message Is Using Form: DD/MM/YY, HH:MM (P|A)M - ..."
    // The DD/MM/YY Thing 
    for(component = i = 0; component < 3; ++component) {
        while(isdigit(*message) && i < 3) 
            num[i++] = *message++;
        if(*message == '/' || *message == ',') {
            message++;
            i = 0; 
            switch(component) {
                case 0:
                    msg->time_sent.tm_yday = atoi(num);
                    break;
                case 1:
                    msg->time_sent.tm_mon = atoi(num);
                    break;
                case 2: 
                    msg->time_sent.tm_year = atoi(num) + whatsapp_year_prefix;
                    break;
                default:
                    fprintf(stderr, "What the actual fuck how ? \n");
                    return NULL;
            }
            num[0] = num[1] = num[2] = '\0';
            continue;

        }
    }
    message++; // Skipping Whitespace

    // The HH:MM Thing 
    num[0] = num[1] = num[2] = '\0';
    for(component = i = 0; component < 2; ++component) {
        while(isdigit(*message) && i < 3)
            num[i++] = *message++;
        if((*message == ':' && message++)|| ( strncmp(message, whatsapp_special_utf_char, 3) == 0 && (message += 3))) { // This is kinda hard to understand so I'll break it into parts 
                                                                                                                        // There are two main parts seprated by an OR statement 
                                                                                                                        // The fist one is if *message == ','. If that's true we only skip it by one  char 
                                                                                                                        // The second one if message is equal to the special unicode char, Then we skip 3 chars. That's It!
            switch(component) {
                case 0: 
                    msg->time_sent.tm_hour = atoi(num);
                    break;
                case 1:
                    msg->time_sent.tm_min = atoi(num);
                    break;
                default:
                    fprintf(stderr, "Ok Stop :( \n");
                    return NULL;
            } 
            num[0] = num[1] = num[2] = '\0';
            i = 0;
            continue;
        } else {
            fprintf(stderr, "Error Parsing Time \n");
            return NULL;
        }

    }

    // Post-Meridean Time 
    if(*message == 'P') 
        msg->time_sent.tm_hour += 12;
    else if(*message != 'A') 
        return NULL; 


    return message+5;
}

// Init Message parsing 
static regex_t exp;
static int is_init = 0;
static int max_word = 0;
static int inv_conv = 3600;
int init_what(struct flags * sys_flags) {
    // The Regular Expression 
    const char *const reg = "[0-9]+\\/[0-9]+\\/[0-9]+, [0-9]+:[0-9]+ (A|P)M - ";
    // Compiles it 
    if(regcomp(&exp, reg, REG_EXTENDED) != 0) {
        fprintf(stderr, "Error: Compiling Regular expression \n");
        exit(EXIT_FAILURE);
    }

    // Setting Max word flag
    max_word = sys_flags->max_words;

    // Setting Interval Between Conversation flag 
    inv_conv = (int) ( sys_flags->conv_interval * 3600.0f);
    is_init = 1;
    return 0;
}

// Check Message 
int is_message(char * message) {
    regmatch_t mtch;
    if(!is_init) {
        fprintf(stderr, "what_stat not initialized properly \n");
        exit(EXIT_FAILURE);
    }
    // Tries Applying Regular Expression to the message 
    if(regexec(&exp, message, 1, &mtch, 0) == 0 && mtch.rm_so == 0)
        return mtch.rm_so;

    return 1;
}


// Parse Username and puts user index in the `msg`
static const char *const end_username = ": "; 
char * parse_username(char * message, struct message * msg) {
    size_t i;
    for(i = 0; strncmp(&message[i], end_username, 2) != 0; ++i)
        ; // Checks Username Length 

    char * username = (char *) malloc(i + 1); // Allocates Storage for username. 
    if(username == NULL) {
        fprintf(stderr, "Error: Out Of Memory. Can't allocate enough storage for username \n");
        return NULL; 
    }

    // Copies To username
    strncpy(username, message, i);
    username[i] = '\0';

    // Allocates Username
    msg->user = store_user(username, i);

    return message+i+2;
}


// Parse A File 
# define MAX_WORD_LEN 256
static size_t word_count = 0;
static size_t conv_count = 0;
static size_t message_count = 0;
static size_t active_hours[HOURS_OF_DAY] = { 0 };
int parse_file(FILE * file_p) {
    if(file_p == NULL) {
        fprintf(stderr, "Error: Accessing File \n");
        exit(EXIT_FAILURE);
    }


    // Get Len Of File 
    fseek(file_p, 0, SEEK_END);
    size_t file_len = ftell(file_p);
    fseek(file_p, 0, SEEK_SET);

    if(file_len == 0) {
        fprintf(stderr, "Error: File Empty \n");
        free_storage();
        fclose(file_p);
        exit(EXIT_FAILURE);
    }


    // Allocates Memory For The File 
    char * content = (char *) malloc(file_len + 1);
    char * ctn_init = content; // To be freed later.
    if(content == NULL) {
        fprintf(stderr, "Error: Out Of Memory. Can't allocate enough memory for file \n");
        free_storage();
        fclose(file_p);
        exit(EXIT_FAILURE);
    }
    content[file_len] = '\0';



    // Copies Data from file 
    if(fread(content, 1, file_len, file_p) != file_len) {
        fprintf(stderr, "Error: Can't copy data from file \n");
        free_storage();
        free(content);
        fclose(file_p);
        exit(EXIT_FAILURE);
    }

    // Skips First Whatsapp Security Message 
    while(*content++ != '\n')
        ;

    // Checks previous time 
    time_t prev_time = 0;

    // Previous user 
    int prev_user = -1;

    // Main Loop
    while(*content != '\0') {
        size_t msg_len;
        for(msg_len = (isdigit(content[1]) ? 2 : 1); content[msg_len] != '\0' && is_message(&content[msg_len]) != 0; ++msg_len)
            ; // Gets Current Message Length. 

        // Allocates space for message 
        char * message_str = (char *) malloc(msg_len + 1);
        if(message_str == NULL) 
        {
            free(ctn_init);
            free_storage();
            fclose(file_p);
            fprintf(stderr, "Error: Out Of Memory. Can't allocate enough memory for message \n");
            exit(EXIT_FAILURE);
        }

        // Copies message 
        strncpy(message_str, content, msg_len - 1);
        message_str[msg_len] = '\0';

        content += msg_len; // Skips To Next Message. 

        // Parse Time && User.
        struct message msg;
        char * time_res = parse_time(message_str, &msg);
        char * message_real = parse_username(time_res, &msg);

        if(time_res == NULL) {
            fprintf(stderr, "Error: Parsing Time. Skipped message %s \n", message_str);
            free(message_str);
            continue;
        }
        if(message_real == NULL) {
            fprintf(stderr, "Error; Parsing User. Skipped message %s \n", message_str);
            free(message_str);
            continue;
        }

        // Parses Message Word By Word 
        for(size_t i = 0; ; ) {
            char word[MAX_WORD_LEN];
            if(!isspace(message_real[i]) && message_real[i] != '\n' && i < MAX_WORD_LEN -  1 && message_real[i] != '\0') {
                word[i] = message_real[i];
                ++i;
                continue;
            }
            word_count++;
            get_user(msg.user)->words_sent++;
            if(max_word != 0 && word_count > max_word) { // Checks For Word Limit. 
                free(message_str);
                free(ctn_init);
                fclose(file_p);
                return 2;
            }
            word[i] = '\0';
            if(i > 0) // Eleminate Single Chars 
                append_word(word, i+1);
            message_real += i + 1;

            // Clearing The String 
            for(int j = 0; j < MAX_WORD_LEN; ++j) 
                word[j] = '\0';

            // Checks For End Of Message 
            if(message_real[i] == '\0')
                break; 
            i = 0;


        }

        // Parses time 
        if(timegm(&msg.time_sent) - prev_time >  inv_conv) {
            conv_count++;
            get_user(msg.user)->conv_start++;
            if(prev_user != -1) 
                get_user(prev_user)->conv_end++;
        }

        // Fills in previous stuff 
        message_count++;
        get_user(msg.user)->mesages_sent++;
        prev_user = msg.user;
        prev_time = timegm(&msg.time_sent);

        // Active Hours 
        active_hours[msg.time_sent.tm_hour]++;
        get_user(msg.user)->active_hours[msg.time_sent.tm_hour]++;

        // Frees stuff 
        free(message_str);
    }

    free(ctn_init);
    fclose(file_p);

    return 0;


}


// Getters 
int get_conv(void) {
    return conv_count;
}
int get_words(void) {
    return word_count;
}
int get_msg(void) {
    return message_count;
}

size_t * get_ac_array(void) {
    return active_hours;
}
