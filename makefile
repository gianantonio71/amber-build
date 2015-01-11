
runme: generated.cpp generated.h
	g++ -I. -Iruntime -O3 -DNDEBUG generated.cpp runtime/*.cpp -o runme

generated.cpp generated.h: main.amber
	./amberc prj.txt

clean:
	rm -f runme generated.cpp generated.h
