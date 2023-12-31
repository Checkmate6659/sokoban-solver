all:
#DEBUG
	# clang++ -Wall -Wextra *.c *.cpp -g3

#PROFILING
	# clang++ -Wall -Wextra *.c *.cpp -g3 -ggdb3 -pg
	# ./a.out
	# gprof -b a.out gmon.out > profile.txt

#LINE PROFILING
	# ++ -fprofile-arcs -ftest-coverage *.c *.cpp
	# ./a.out
	# gcov *.c *.cpp
	# rm *.gcda *.gcno

#RELEASE
	clang++ -Wall -Wextra *.c *.cpp -O3 -Ofast -flto -march=native -s -DNDEBUG -fno-signed-zeros -funroll-loops -fomit-frame-pointer -fno-stack-protector
