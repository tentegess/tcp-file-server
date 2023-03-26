# tcp-file-server-
Server that allows multiple clients to both upload and download files at the same time, written in c++.

The client, after the connection is established, sends information to the server about the planned activity
("UPLOAD" - upload a file to the server, "DOWNLOAD: filename" - download file from server). 
