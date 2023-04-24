# include "what_types.h"



const char *const usage_str = "Usage: what_stat FILENAME [PARAMS] [OPTIONS]\n"\
                              "Parameters\n"\
                              "  -m { int }\t Set a maximum word limit. Default: Infinite\n"\
                              "  -u { int }\t Set a maximum user limit. Default: 256\n" \
                              "  -i {float}\t Set Interval between two conversations in hours. Default: 1.0h\n"\
                              "Options\n"\
                              "  -e {char}\t Search For a specific word \n"\
                              "  -g {none}\t Show global results too (if combined with `-e`) \n";

struct flags * parse_flags(int argc, char ** argv) {
    // if(argc < 2 || argc > 11) {
    //    puts(usage_str);
    //    exit(EXIT_FAILURE);
    //}

    size_t i;
    // Allocating Struct 
    struct flags * sys_flags = (struct flags *) malloc(sizeof(*sys_flags));
    if(sys_flags == NULL) {
        fprintf(stderr, "Error allocating memory to parse flags \n");
        exit(EXIT_FAILURE);
    }
    
    // Setting Defaults
    sys_flags->max_users = 256;
    sys_flags->max_words = 0;
    sys_flags->op = WHAT_GENERAL;
    sys_flags->conv_interval = 1.0f;
    sys_flags->spec_word = NULL;
    sys_flags->filename = NULL;

    static char e_or_g_before = 0, is_filename_before = 0;

    for(i = 1; i < argc; ++i) {
        if(*argv[i] == '-') {
            switch(*++argv[i]) {
                case 'm':
                    sys_flags->max_words = atoi(argv[++i]);
                    if(sys_flags->max_words == 0) {
                        free(sys_flags);
                        puts(usage_str);
                        exit(EXIT_FAILURE);
                    }
                    break;
                
                case 'u' :
                    sys_flags->max_users = atoi(argv[++i]);
                    if(sys_flags->max_users == 0) {
                        free(sys_flags);
                        puts(usage_str);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'i' :
                    sys_flags->conv_interval = atof(argv[++i]);
                    if(sys_flags->conv_interval == .0f) {
                        free(sys_flags);
                        puts(usage_str);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'e':
                    e_or_g_before++;
                    sys_flags->spec_word = argv[++i];
                    break;
                case 'g':
                    e_or_g_before++;
                    break;
                default:
                    free(sys_flags);
                    puts(usage_str);
                    exit(EXIT_FAILURE);
                    break;
            }
        } else {
           if(is_filename_before) {
            free(sys_flags);
            puts(usage_str);
            exit(EXIT_FAILURE);
           }
           is_filename_before++;

           sys_flags->filename = argv[i];
           continue;
        }
    }
    if(sys_flags->filename == NULL) {
        // free(sys_flags);
        // puts(usage_str);
        // exit(EXIT_FAILURE);
        sys_flags->filename = "some_chat.txt";
    }
    
    if(sys_flags->spec_word != NULL && e_or_g_before == 1)
        sys_flags->op = WHAT_SPECIFIC;
    else if(e_or_g_before == 2)
        sys_flags->op = WHAT_COMBINED;
    else 
        sys_flags->op = WHAT_GENERAL;

    return sys_flags;
}
