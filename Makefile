FloorServer:
	g++ -g -Wall -o FloorServer main.cpp -lboost_thread -lboost_date_time -lboost_system

clean:
	rm -f FloorServer 
