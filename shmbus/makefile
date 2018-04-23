total:send recv busd

send:test_send.cpp shmbus.cpp
	g++ test_send.cpp shmbus.cpp -o send -I./ -I../include -g -std=c++11


recv:test_recv.cpp shmbus.cpp
	g++ test_recv.cpp shmbus.cpp -o recv -I./ -I../include -g -std=c++11


busd:bus.cpp 
	g++ bus.cpp -I../include -I./ -o bus -std=c++11

so:
	g++ shmbus.cpp -I./ -I../include -fPIC -shared -o libshmbus.so -std=c++11

a:
	g++ shmbus.cpp -I./ -I../include -c -o shmbus.o -std=c++11
	ar rcs libshmbus.a shmbus.o
	rm shmbus.o

clear:
	rm send recv bus libshmbus*
