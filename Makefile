all:
#DEBUG
	# clang -Wall -Wextra *.c -g3

#PROFILING
	# clang -Wall -Wextra *.c -g3 -ggdb3 -pg
	# ./a.out
	# gprof -b a.out gmon.out > profile.txt

#RELEASE
	clang++ -Wall -Wextra *.c *.cpp -O3 -Ofast -flto -march=native -s -DNDEBUG -fno-signed-zeros -funroll-loops -fomit-frame-pointer -fno-stack-protector
