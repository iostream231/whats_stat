# include "./what_types.h"
# include <string.h>



// Storage Arrays 
static struct user * users; 
static struct word_t * words;
static int max_users = 0;

// Keeping Pointers To be freed 
struct flags * ctn_flags;


// Initializes and allocates necessary storage 
int init_storage(struct flags * sys_flags) {
        // Allocates User storage 
    users = (struct user *) malloc(sizeof(struct user) * sys_flags->max_users); 
    if(users == NULL) {
        fprintf(stderr, "Out Of Memory. Can't allocate enough storage for users \n");
        exit(EXIT_FAILURE);
    }

    max_users = sys_flags->max_users;
    ctn_flags = sys_flags;

    return 0;
}


// Stores new user or returns index of a user with similar username
static int count = 0;
int store_user(char * username, size_t len) {
    if(max_users == 0) {
        fprintf(stderr, "Storage not initialized properly \n");
        exit(EXIT_FAILURE);
    }

    for(size_t i = 0; i < count; ++i) 
        if(strncmp(username, users[i].username, len) == 0)
            return i;

    if(count >= max_users) {
        fprintf(stderr, "Max user count reached  \n");
        return -1;
    }
    
    users[count].username = strndup(username, len);
    for(size_t i = 0; i < HOURS_OF_DAY; ++i) 
        users[count].active_hours[i] = 0; // Initializes Active Hours array to 0
    return count++;
}

// Gets User at a specific index 
struct user * get_user(int user) {
    if(user >= count) 
        return NULL;

    return &users[user];
}

// Appends Word To Word Tree
static struct word_t * append_word_storage(struct word_t * word_t, char * word, size_t len) {
    // No Word Was found with that name. Make New 
    if(word_t == NULL) {
        word_t = (struct word_t *) malloc(sizeof(*word_t));
        if(word_t == NULL) {
            fprintf(stderr, "Error : Out of memory. Can't allocate enough memory for word \n");
        } else {
            word_t->word = strndup(word, len);
            word_t->count = 1;
            word_t->left = word_t->right = NULL;
        }
    } else if (strcmp(word_t->word, word) > 0)  // Word is at right branch of the tree 
        word_t->right = append_word_storage(word_t->right, word,  len);
    else if(strcmp(word_t->word, word) < 0) // Word is at left branch of the tree 
        word_t->left = append_word_storage(word_t->left, word, len);
    else 
        word_t->count++; // Word Already Exists 

    return word_t; // Returns word 
}

// Append Word To Storage (an simplified version of the function above)
int append_word(char * word, size_t len) {
   words = append_word_storage(words, word, len);
   return 0;
}

// Look Up For A Word In The tree 
static struct word_t * lookup_word(struct word_t * word_t, char * word) {
    if(word_t == NULL) 
        return word_t;
    else if (strcmp(word_t->word, word) > 0) 
        return lookup_word(word_t->right, word);
    else if (strcmp(word_t->word, word) < 0) 
        return lookup_word(word_t->left, word);
    else 
        return word_t;
}
// Search For A Word (Simplified Version of the function above) 
struct word_t * search_word(char * word) {
    return lookup_word(words, word);
}

// Recursively free the Words tree 
static int free_tree(struct word_t* word_t) {
    if(word_t != NULL) {
        free_tree(word_t->left);
        free_tree(word_t->right);
        free(word_t->word);
        free(word_t);
    }
    return 0;
}
// Frees Storage
int free_storage(void) {
    free(users);
    free(ctn_flags);
    free_tree(words);
    return 0;
    
}

// Get Effective users 
struct user * get_effective_users(size_t * len) {
    *len = count;
    return users;
}

// The index of the largest element in an array 
size_t largest(size_t array[], size_t len) {
    size_t max = 0;

    for(size_t i = 0; i < len; ++i) 
        if(array[i] > array[max])
            max = i;
    return max;
}
