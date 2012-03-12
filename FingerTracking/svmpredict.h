

int svm_predict(char* input_f_name, char *model_f_name, char *output_f_name);
void predict(FILE *input, FILE *output);
void exit_input_error(int line_num);
void exit_with_help();