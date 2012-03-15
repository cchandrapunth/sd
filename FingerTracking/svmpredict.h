

//accuracy estimation
int svm_predict(char* input_f_name, char *model_f_name, char *output_f_name);
void predict(FILE *input, FILE *output);
void exit_input_error(int line_num);
void exit_with_help();

//use in real time
int init_predict(char *model_f_name);
int svm_rt_predict(Pair* p, int size);