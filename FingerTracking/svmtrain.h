

int svm_train(char *input_f_name, char *model_f_name);
void set_default();
void read_problem(const char *filename);
void do_cross_validation();

void exit_with_help();
void exit_input_error(int line_num);
