void foo(){}
void foo(){}// function redefinition
const int a=1;
int b[a];// constexpr_expected && variable length array declaration is not allowed
int g;
const int b[2] = {g};// redefinition & initializer element is not a compile-time constant
void main(){
	c = 1;// var is not define, use before define
	a = 2;// cannot assign to variable with const-qualified type 'const int'
	int d = a[2];// subscripted variable is not an array
	g = b;// array name cannot be used as value directly
	int e[-2];// declared as an array with a negative size
	int f[2]={1,2,3};// excess elements in array initializer
	bar();// function is not declared
}
int c = 3 / (3 - 3 + a);//initializer should be const expression && since a is ignored, 3 is divided by 0
