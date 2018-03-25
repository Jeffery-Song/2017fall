q1:test1.c
	gcc test.c -o q1
q2:test2.c
	gcc test1.c test2.c -o q2
	gcc test1.c test2.c -S
runq1:q1
	./q1
runq2:q2
	./q2
