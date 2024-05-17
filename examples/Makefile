LINKED_LIBRARY = -lpthread
STANDARD = c++11

basic : basic.cpp
	g++ -std=${STANDARD} basic.cpp ${LINKED_LIBRARY} -o basic.out
	./basic.out

timeout: timeout.cpp
	g++ -std=${STANDARD} timeout.cpp ${LINKED_LIBRARY} -o timeout.out
	./timeout.out

timer: timer.cpp
	g++ -std=${STANDARD} timer.cpp ${LINKED_LIBRARY} -o timer.out
	./timer.out

clean:
	rm -rf basic.out timeout.out timer.out