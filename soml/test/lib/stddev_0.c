uint32_t stddev_0_input_0 [] = { 1, 2, 3 };
double stddev_0_output_0 [] = { 1.0, 1.0 };

TestVector** stddev_0_tv_input  = (TestVector**)malloc(1 * sizeof(TestVector*));
if (!stddev_0_tv_input) return NULL;
TestVector** stddev_0_tv_output = (TestVector**)malloc(1 * sizeof(TestVector*));
if (!stddev_0_tv_output)
{
 free(stddev_0_tv_input);
 return NULL;
}
stddev_0_tv_input[0] = make_test_vector (stddev_0_input_0, OML_INT32_VALUE, 3);
stddev_0_tv_output[0] = make_test_vector (stddev_0_output_0, OML_DOUBLE_VALUE, 2);

TestData* stddev_0_data = (TestData*)malloc(sizeof(TestData));
if (!stddev_0_data)
{
 free(stddev_0_tv_input); free(stddev_0_tv_output);
 return NULL;
}
stddev_0_data->count = 1;
stddev_0_data->inputs = stddev_0_tv_input;
stddev_0_data->outputs = stddev_0_tv_output;
return stddev_0_data;
