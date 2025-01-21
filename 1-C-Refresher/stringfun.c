#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
void reverse_string(char *, int);
void print_words(char *, int);
void replace_word(char *buff, int len, char *search, char *replace);

int setup_buff(char *buff, char *user_str, int len){
    char *src = user_str;
    char *dst = buff;
    int char_count = 0;
    int was_space = 1;
    if (user_str == NULL || *user_str == '\0') {
        return -2;
    }
    while (*src != '\0') {
	if (*src == ' ' || *src == '\t') {
	    char *next = src + 1;
	    while (*next == ' ' || *next == '\t') next++;
	    if (!was_space && *next != '\0') {
	 	if (char_count >= len) return -1;
		*dst = ' ';
		dst++;
		char_count++;
		was_space = 1;
	    }
	} else {
	     if (char_count >= len) return -1;
             *dst = *src;
             dst++;
             char_count++;
	     was_space = 0;
	}

        src++;
    }
    while (char_count < len) {
        *dst = '.';
       	dst++;
        char_count++;
    }
    return char_count;
}

void print_buff(char *buff, int len){
    printf("Buffer:  [");
    int i = 0;
    while (i < len && buff[i] != '.') {
        putchar(buff[i]);
        i++;
    }
    while (i < len) {
        putchar(buff[i]);
        i++;
    }
    printf("]\n");
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    int word_count = 0;
    int in_word = 0;

    if (str_len == 0 || buff[0] == '\0') {
	return 0;
    }

    for (int i = 0; i < str_len && i < len; i++) {
        if (buff[i] != ' ' && buff[i] != '.' && buff[i] != '\t') {
            if (in_word == 0) {
                word_count++;
                in_word = 1;
            }
        } else {
            in_word = 0;
        }
    }

    return word_count;
}

void reverse_string(char *buff, int str_len) {
    char *start = buff;
    char *end = buff + str_len - 1;
    while (end >= start && *end == '.') {
        end--;
    }
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}

void print_words(char *buff, int str_len) {
    printf("Word Print\n");
    printf("----------\n");

    int word_start = -1;
    int word_count = 0;

    for (int i = 0; i <= str_len; i++) {
        if (buff[i] == ' ' || buff[i] == '.' || buff[i] == '\0') {
            if (word_start!= -1) {
                printf("%d. ", word_count + 1);
                for (int j = word_start; j < i; j++) {
                    putchar(buff[j]);
                }
                printf("(%d)\n", i - word_start);
		word_count++;
		word_start = -1;
            }

        } else {
	    if (word_start == -1) {
	    	word_start = i;
	    }
	}
    }
    if (word_start != -1) {
        printf("%d. ", word_count + 1);
        for (int j = word_start; j < str_len; j++) {
            putchar(buff[j]);
        }
        printf("(%d)\n", str_len - word_start);
    }
    printf("\n");
    printf("Number of words returned: %d\n", word_count);
}

void replace_word(char *buff, int len, char *search, char *replace) {
    char temp[len];
    char *src = buff;
    char *dst = temp;
    int search_len = 0;
    int replace_len = 0;

    while (search[search_len] != '\0') search_len++;
    while (replace[replace_len] != '\0') replace_len++;

    while (src < buff + len && *src != '\0') {
        if (strncmp(src, search, search_len) == 0) {
            for (int i = 0; i < replace_len; i++) {
                *dst++ = replace[i];
            }
            src += search_len;
        } else {
            *dst++ = *src++;
        }
    }

    *dst = '\0';
    int modified_len = dst - temp;
    memcpy(buff, temp, modified_len);
    buff[modified_len] = '\0';
}

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //It checks if the program has enough arguments, and if the program didn't have enough arguments, it will show the usage and it will exit.(*argv[1] != '-') part make sure that first argument starts with -. 
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //if statement below ensures that a string and the option flag are gathered. If any of them is missing, the program will print the usage and exit so that there won't be errors.
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    buff = malloc(BUFFER_SZ);
    if (buff == NULL) {
    	printf("Error: Malloc failed.\n");
    	exit(99);
    }


    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
	case 'r':
    	    reverse_string(buff, user_str_len);
    	    break;

	case 'w':
    	    print_words(buff, user_str_len);
    	    break;
	case 'x':
	    if (argc <= 5) {
            	printf("Not Implemented!\n");
            	break;
            }
	    replace_word(buff, BUFFER_SZ, argv[3], argv[4]);
    	    break;

        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//Providing both the pointer and the length is a good practice because this way we ensure the error prevention. This way function can work with any size buffer and prevent the memory access that is beyond the allocated buffer, and reusing the code becomes more easier.

