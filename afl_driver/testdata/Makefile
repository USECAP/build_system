all:
	clang++ target.cpp -fsanitize-coverage=trace-pc-guard -L${AFL_PATH} -lafl_engine -o fuzzer

clean:
	rm fuzzer
