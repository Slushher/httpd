.PHONY: cmake compile run clean

cmake:
	cmake -B cmake

compile:
	cmake --build cmake

run:
	./bin/httpd

clean:
	rm cmake -rf