total:send recv busd

send:test_send.cpp shmbus.cpp
	g++ test_send.cpp shmbus.cpp -o send -I./ -I../include -g -std=c++11


recv:test_recv.cpp shmbus.cpp
	g++ test_recv.cpp shmbus.cpp -o recv -I./ -I../include -g -std=c++11


busd:bus.cpp 
	g++ bus.cpp -I../include -I./ -o bus -std=c++11

clear:
	rm send recv bus
