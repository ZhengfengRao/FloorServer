FloorServer
===========
A simple server framework using epoll. Implemented the following features:
1.epoll accept connections.
2.thread pool to handle all connections(receive meesge to a buffer).
3.close inactive connections to avoid ddos attack. 
4.log module

All You have to do is:
1.implement you logic code, because the message is received to a buffer for every connection, you should handle it with your bussiness.


TODO:
1.dynamiclly load configure(log, connection params...), change settings without restart the programe. 
2.graceful exit when we need to 
3.deny ip.add hostile ip(frequently connect, inactive connect) to blacklist.

Zhengfeng Rao
2013-3-12

