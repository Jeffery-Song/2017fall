int fib_n;
int fib_result;

void fib()
{
	int n = fib_n;
	if(fib_n <= 1)
		fib_result = 1;
	else
	{
		fib_n = n - 1;
		fib();
		int result = fib_result;
		fib_n = n - 2;
		fib();
		fib_result = result + fib_result;
	}
}


void main() {
	input();
	fib_n = input_var;
	fib();
	output_var = fib_result;
	output();
}