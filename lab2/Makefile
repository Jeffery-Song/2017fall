fib-gen.ll: fib-gen.cpp
	g++ fib-gen.cpp `llvm-config --cxxflags --ldflags --libs --system-libs` -o fib-gen
	./fib-gen > fib-gen.ll

rungenir:fib-gen.ll
	lli fib-gen.ll
runhandmadeir:fib.ll
	lli fib.ll

clean:
	-rm fib-gen.ll
	-rm fib-gen
