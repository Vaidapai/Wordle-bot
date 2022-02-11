//
//  wordle-bot.c
//  Wordle-bot
//
//  Created by Vaishak Krishna on 2/9/22.
//
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#define NUM_LETTERS = 5;
#define NUM_GUESSES = 6;
#define LENGTH_OF_WORDS = 5757;
#define NUM_PATTERNS = 243;
typedef struct Word{
    char *word;
    double score;
} Word;
typedef struct Word_list {
    Word *words;
    int num_words;
} Word_list;

int* order_of_pattern_traversal(char *pattern){
    int *result = malloc(sizeof(int)*5);
    int pointer = 0;
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 5; j++){
            //checking for g
            if (i == 0 && pattern[j] == 'G' ){
                result[pointer] = j;
                pointer++;
            }
            //checking for y
            if (i == 1 && pattern[j] == 'Y'){
                result[pointer] = j;
                pointer++;
            }
            //checking for r
            if (i == 2 && pattern[j] == 'R'){
                result[pointer] = j;
                pointer++;
            }
        }
        
    }
    return result;
}

Word_list copy_word_list(Word_list w) {
    Word_list result;
    result.words = malloc(w.num_words*sizeof(struct Word));
    result.num_words = w.num_words;
    for (int i = 0; i < w.num_words; i++){
        result.words[i].word = malloc(sizeof(char)*6);
        strcpy(result.words[i].word, w.words[i].word);
        result.words[i].score = w.words[i].score;
    }
    return result;
}

double sigmoid(int word){
    return 1.0 - 1.0/(1 + exp((word-3000)/100));
}

char** create_all_patterns() {
    char **all_patterns = (char **) malloc(sizeof(char *) * 243);
    char pattern_letters[] = {'R', 'Y', 'G'};
    int counter = 0;
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            for (int k = 0; k < 3; k++){
                for (int l = 0; l < 3; l++){
                    for (int m = 0; m < 3; m++){
                        all_patterns[counter] = malloc(sizeof(char) * (6));
                        all_patterns[counter][0] = pattern_letters[i];
                        all_patterns[counter][1] = pattern_letters[j];
                        all_patterns[counter][2] = pattern_letters[k];
                        all_patterns[counter][3] = pattern_letters[l];
                        all_patterns[counter][4] = pattern_letters[m];
                        all_patterns[counter][5] = '\0';
                        counter++;
                    }
                }
            }
        }
    }
    return all_patterns;
}

Word_list read_words() {
    char buff[6];
    Word_list words;
    words.words = malloc(sizeof(struct Word) * 5757);
    words.num_words = 5757;
    FILE* fp = fopen("/Users/vaishakkrishna/Documents/Swift Projects/Wordle-bot/Wordle-Bot/Wordle-Bot/words_by_freq.txt", "r");
    for (int i = 0; i < 5757; i++) {
        fscanf(fp, "%s", buff);
        words.words[i].word = malloc(6*sizeof(char));
        strcpy(words.words[i].word, buff);
        words.words[i].score = sigmoid(i);
//        printf("word: %s score: %f",  words.words[i].word,  words.words[i].score);
    }
    fclose(fp);
    
    return words;
}
int letter_in_word(char letter, char* word, char start_index) {
    for (int i = start_index; i < 5; i++) {
        if (word[i] == letter) {
            return 1;
        }
    }
    return 0;
}
int index_of_lettter(char *word, char letter){
    for (int i = 0; i < strlen(word); i++){
        if (word[i] == letter){
            return i;
        }
    }
    return -1;
}

double info_gain_times_probability(Word_list reduced_words, int size_of_reduced_words, char * this_word, char * pattern) {
    double words_with_pattern = 0;
    Word word;
    int *order = order_of_pattern_traversal(pattern);
    for (int i = 0; i < reduced_words.num_words; i++) {
        
        word = reduced_words.words[i];
        if (word.word == NULL){
            continue;
        }
        char new_word[6];
        strcpy(new_word, word.word);
        for (int j = 0; j < 5; j++) {
            int letter = order[j];
            int index = index_of_lettter(new_word, this_word[letter]);
            if (pattern[letter] == 'R' ) {
                if (index >= 0) {
                    break;
                }
            }
            else if (pattern[letter] == 'Y') {
                if (index == -1){
                    break;
                }
            }
            else if (pattern[letter] == 'G') {
                if (this_word[letter] != word.word[letter]){
                    break;
                }
            }
            if (index >=0){
                for (int k = index; k < 5-j; k++){
                    new_word[k] = new_word[k+1];
                }
            }
            
            if (j == 4) {
                
                words_with_pattern += 1;
                reduced_words.words[i].word = NULL;
            }
        }
    }
    return (words_with_pattern == 0) ? 0 : (log(size_of_reduced_words/words_with_pattern)/log(2))*(words_with_pattern/size_of_reduced_words);
}

double expected_information(Word_list reduced_words, char *this_word) {
    double EV_info = 0;
    char **all_patterns = create_all_patterns();
    
    Word_list reduced_words_cpy = copy_word_list(reduced_words);
    for (int i = 0; i < 243; i++) {
        EV_info += info_gain_times_probability(reduced_words_cpy, reduced_words.num_words, this_word, all_patterns[i]);
    }
    
    return EV_info;
}


Word_list reduce_words(Word_list words, char *this_word, char *pattern) {
    Word_list reduced_words;
    reduced_words.words = malloc(sizeof(struct Word) * words.num_words);
    reduced_words.num_words = 0;
    
    char *word;
    int *order = order_of_pattern_traversal(pattern);
    for (int i = 0; i < words.num_words; i++) {
        word = words.words[i].word;
        char new_word[6];
        strcpy(new_word, word);
        
        for (int j = 0; j < 5; j++) {
            int letter = order[j];
            int index = index_of_lettter(new_word, this_word[letter]);
            if (pattern[letter] == 'R' ) {
                if (index >= 0) {
                    break;
                }
            }
            else if (pattern[letter] == 'Y') {
                if (index == -1){
                    break;
                }
            }
            else if (pattern[letter] == 'G') {
                if (this_word[letter] != word[letter]){
                    break;
                }
            }
            if (index >=0){
                for (int k = index; k < 5-j; k++){
                    new_word[k] = new_word[k+1];
                }
            }
            
            
            if (j == 4) {
                reduced_words.words[reduced_words.num_words].word = malloc(sizeof(char) * (5 + 1));
                strcpy(reduced_words.words[reduced_words.num_words].word, word);
                
                reduced_words.num_words++;
            }
        }
    }
    
    for (int i = 0; i < words.num_words; i++){
        free(words.words[i].word);
    }
    free(words.words);
    return reduced_words;
}
char *suggest_word(Word_list words, Word_list reduced_words) {
    if (reduced_words.num_words == 1){
        return reduced_words.words[0].word;
    }
    if (reduced_words.num_words <= 5){
        printf("Top guesses: ");
        for (int j = 0; j < reduced_words.num_words; j++){
            printf("%s, ", reduced_words.words[j].word);
        }
    }
    double max_info = 0;
    char *max_word = 0;
    for (int i = 0; i < 5757; i++) {
        double info = expected_information(reduced_words, words.words[i].word);
        if (info > max_info) {
            max_info = info;
            max_word = words.words[i].word;
        }
    }
    return max_word;
}

void main_loop(){
    char *guess = "crane";
    Word_list words = read_words();
    Word_list reduced_words = read_words();
    
    for (int i = 0; i < 6; i++){
        printf("%s\n", guess);
        char *input = malloc(sizeof(char)*6);
        scanf("%s",input);
        if (input[0] == 'G' && input[1] == 'G' && input[2] == 'G' && input[3] == 'G' && input[4] == 'G'){
            printf("good job\n");
            return;
        }
        reduced_words = reduce_words(reduced_words, guess, input);
        char *suggested_word = suggest_word(words, reduced_words);
        guess = suggested_word;
        
    }
    printf("You suck\n");
}
int main(int argc, const char * argv[]) {
    main_loop();
    return 0;
}
