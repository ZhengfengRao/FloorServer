FloorServer
===========
A simple server framework using epoll. Implemented the following features:
1.epoll accept connections.
2.thread pool to handle all connections(receive meesge to a buffer).
3.close inactive connections to avoid ddos attack. 
4.log module
5.dynamiclly load configure(log, connection params...), change settings without restart the programe. 
6.graceful exit when we need to 

All You have to do is:
1.implement you logic code at WorkThread.h:97, because the message is received to a buffer for every connection, you should handle it with your bussiness.


TODO:
1.deny ip.add hostile ip(frequently connect, inactive connect) to blacklist.

Zhengfeng Rao
2013-4-24

