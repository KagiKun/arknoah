total:send recv busd

send:test_send.cpp shmbus.cpp
	g++ test_send.cpp shmbus.cpp -o send -I./ -I../include -g -std=c++11


recv:test_recv.cpp shmbus.cpp
	g++ test_recv.cpp shmbus.cpp -o recv -I./ -I../include -g -std=c++11


busd:busd.cpp 
	g++ busd.cpp -I../include -I./ -o bus 

clear:
	rm send recv bus
