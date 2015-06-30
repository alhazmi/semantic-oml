uint32_t stddev_1_input_0 [] = { 1, 2, 3 };
uint32_t stddev_1_input_1 [] = { 2, 3, 4 };
uint32_t stddev_1_input_2 [] = { -7, -2, -11 };
double stddev_1_output_0 [] = { 1.0, 1.0 };
double stddev_1_output_1 [] = { 1.0, 1.0 };
double stddev_1_output_2 [] = { 4.50924975282, 20.3333333333 };

TestVector** stddev_1_tv_input  = (TestVector**)malloc(3 * sizeof(TestVector*));
if (!stddev_1_tv_input) return NULL;
TestVector** stddev_1_tv_output = (TestVector**)malloc(3 * sizeof(TestVector*));
if (!stddev_1_tv_output)
{
 free(stddev_1_tv_input);
 return NULL;
}
stddev_1_tv_input[0] = make_test_vector (stddev_1_input_0, OML_INT32_VALUE, 3);
stddev_1_tv_input[1] = make_test_vector (stddev_1_input_1, OML_INT32_VALUE, 3);
stddev_1_tv_input[2] = make_test_vector (stddev_1_input_2, OML_INT32_VALUE, 3);
stddev_1_tv_output[0] = make_test_vector (stddev_1_output_0, OML_DOUBLE_VALUE, 2);
stddev_1_tv_output[1] = make_test_vector (stddev_1_output_1, OML_DOUBLE_VALUE, 2);
stddev_1_tv_output[2] = make_test_vector (stddev_1_output_2, OML_DOUBLE_VALUE, 2);

TestData* stddev_1_data = (TestData*)malloc(sizeof(TestData));
if (!stddev_1_data)
{
 free(stddev_1_tv_input); free(stddev_1_tv_output);
 return NULL;
}
stddev_1_data->count = 3;
stddev_1_data->inputs = stddev_1_tv_input;
stddev_1_data->outputs = stddev_1_tv_output;
return stddev_1_data;
