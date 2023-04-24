# ifndef WHAT_STAT 
    # define WHAT_STAT 1
    # include <stdio.h>
    # include <stdlib.h>
    # include <time.h>

    enum operation {WHAT_GENERAL, WHAT_SPECIFIC, WHAT_COMBINED};


    # define HOURS_OF_DAY 24
    struct user {
        char * username; // Userame. Must be null terminated.
                         
        size_t words_sent; // Total Words Sent by user 
        size_t mesages_sent; // Total Messages Sent by user 

        size_t conv_start; // Total Conversations Started 
        size_t conv_end; // Total Conversations Ended 
    
        size_t active_hours[HOURS_OF_DAY]; // Count of messages sent in each specific hour.
    };

    struct message {
        struct tm time_sent; // The time of sending the message 
        int user; // The message's author's index on the users list.
    };
    
    struct flags {
        int max_users; // Max users 

        size_t max_words; // Max words 
        
        float conv_interval; // Interval between Conversations

        enum operation op; // Options Possible 
        char * spec_word;

        char * filename; // Filename
    };
    
    struct word_t {
        char * word; // Null terminated word string 

        size_t count; // Usage of the word 

        struct word_t * left;
        struct word_t * right; // Left And Right Components of the tree
    };
    
# endif
