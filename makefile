default: chatclient chatserve.class ServerSender.class

chatclient: chatclient.c
	gcc chatclient.c -lncurses -o chatclient

chatserve.class: chatserve.java
	javac chatserve.java

ServerSender.class: ServerSender.java
	javac ServerSender.java
