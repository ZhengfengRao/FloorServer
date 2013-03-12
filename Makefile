FloorServer:
	g++ -g  -lboost_thread -lboost_date_time  main.cpp -o FloorServer

clean:
	rm -f FloorServer 
