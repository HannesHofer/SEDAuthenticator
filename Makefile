server: devurandom.o server.o
	$(CXX) -o $@ server.o devurandom.o -lnacl
