all:
	g++ dispatchd.cpp dispatch_config.cpp main.cpp protocol.pb.cpp -o dispatchd -I./ -I../include -I../shmbus -L../shmbus -lshmbus -lev -lglog -lpthread -lprotobuf -g -std=c++11


clear:
	rm dispatchd
