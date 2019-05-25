
test: *.cpp *.hpp
	g++ -std=c++14 -DTEST -I./ SeqRecorder.cpp -o test 
	./test | grep 'Done'
	
clean:
	rm -rf test*
