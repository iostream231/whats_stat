# include "include/what_types.h"
# include "include/what_args.h"
# include "include/what_message.h"





enum metric {MESSAGE_SENT, WORD_SENT, CONV_START, CONV_END};
static const char *const block_uni = "█";
int draw_histogram(struct user * users, size_t len, enum metric mtc) {
    // Init Variables 
    float user_perc = .0f, init_perc = user_perc;
    size_t len_s = 0;


    for(size_t i = 0; i < len; ++i) {
        // Initializes Stuff 
        char user_drawname[16];

        // Copies Data 
        strncpy(user_drawname, users[i].username, 16);
        user_drawname[15] = '\0';
        for(size_t j = strlen(users[i].username);j < 16 ;++j) {
            user_drawname[j] = ' '; 
        }



        // Print User 
        printf("%s|", user_drawname);

        // Calculates The Percentage
        switch(mtc) {
            case MESSAGE_SENT :
                // Calculates Percentage 
                user_perc = (float) users[i].mesages_sent / (float) get_msg();
                break;
                // Same Thing For The rest. 
            case WORD_SENT :
                user_perc = (float) users[i].words_sent / (float) get_words();
                break;
            case CONV_START :
                user_perc = (float) users[i].conv_start / (float) get_conv();
                break;
            case CONV_END : 
                user_perc = (float) users[i].conv_end / (float) get_conv();
                break;
        }

        // Saves Percentage 
        init_perc = user_perc;


        // Draws Histogram 
        while(user_perc > .01f) {
            printf("%s", block_uni);
            user_perc -= .01f;
        }

        // Percentage
        printf("%.1f%%", init_perc * 100.0f);
        
        // Actual Number 
        switch(mtc) {
            case MESSAGE_SENT :
                printf("\t(%lu)\n", users[i].mesages_sent);
                break;
            case WORD_SENT :
                printf("\t(%lu)\n", users[i].words_sent);
                break;
            case CONV_START :
                printf("\t(%lu)\n", users[i].conv_start);
                break;
            case CONV_END :
                printf("\t(%lu)\n", users[i].conv_end);
                break;
        }
    }
    return 0;
};

const char * const sep = "\n------------------------------------\n";
int main(int argc, char ** argv) {
    struct flags * sys_flags = parse_flags(argc, argv);

    init_what(sys_flags);
    init_storage(sys_flags);

    FILE * file_p = fopen(sys_flags->filename, "r");

    if(file_p == NULL) {
        free_storage();
        fprintf(stderr, "Error: Opening file %s \n", sys_flags->filename);
        exit(EXIT_FAILURE);
    }

    parse_file(file_p);

    size_t len = 0;
    struct user * total_users = get_effective_users(&len);
    if(total_users == NULL) 
        return -1;

    // So I figured out that end conversation counter will always be n-1 than the total conv counter.
    // So as a quick fast fix ;)
    total_users[0].conv_end++;

    // Print Stuff 
    if(sys_flags->op == WHAT_COMBINED || sys_flags->op == WHAT_GENERAL) {
        // General Info 
        puts("========== General Info ==========");
        printf("\tfilename : %s \n", sys_flags->filename);
        printf("\tTotal Messages : %i\n", get_msg());
        printf("\tTotal Users : %lu\n", len);
        printf("\tTotal Words : %i\n", get_words());
        printf("\tTotal Conversations: %i\n", get_conv());

        // Histograms 
        puts("========== Messages Sent ============");
        draw_histogram(total_users, len, MESSAGE_SENT);

        puts(sep);

        puts("========== Words Sent ==========");
        draw_histogram(total_users, len, WORD_SENT);

        puts(sep);

        puts("========== Conversations Started ==========");
        draw_histogram(total_users, len, CONV_START);

        puts(sep);

        puts("========== Conversations Ended ==========");
        draw_histogram(total_users, len, CONV_END);

        puts(sep);
    
        puts("========== Active Hours ==========");
        size_t * active_hours = get_ac_array();
        size_t most_active = largest(get_ac_array(), HOURS_OF_DAY);
        printf("Most Active Hour: %lu. (%lu messages sent. %.1f%% ). \n", most_active, active_hours[most_active], ((float) active_hours[most_active] / (float) get_msg()) * 100.0f);
        for(size_t i = 0; i < len; ++i) {
            most_active = largest(total_users[i].active_hours, HOURS_OF_DAY);
            printf("Most Active Hour For %s : %lu. (%lu messages sent. %.1f%% ). \n",
                     total_users[i].username,
                     most_active, total_users[i].active_hours[most_active], ((float) active_hours[most_active] / (float) total_users[i].mesages_sent) * 100.0f);

        }

        
    }
    if(sys_flags->op == WHAT_COMBINED || sys_flags->op == WHAT_SPECIFIC) {
        puts(sep);
        printf("========== LookUp Word %s ==========\n", sys_flags->spec_word);
        struct word_t * word = search_word(sys_flags->spec_word);
        if(word == NULL) 
            printf("Usage 0. (0.000%%)\n");
        else 
            printf("Usage %lu. (%.3f%%)\n", word->count, ((float) word->count / (float) get_words()) * 100.0f);
    }

    puts(sep);
    
    free_storage();

    return 0;
}
