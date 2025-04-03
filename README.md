# Asynchronous HTTP Requests

What is this about?
This Package enables your Project to handle HTTP Requests in an asynchronous way.

# What does this Repository contain?

## async_http_requests - ahr

C Code.
libahr is written in C. It builds on libcurl and provides some additional utility Features.

If you want to learn more about libahr take a look at async_http_requests/.

## python_async_http_requests - pyahr

Python Code.
This is a Wrapper for libahr in Python. pyahr allows you to use libahr from within your Pythonscripts.

If you want to learn more about pyahr take a look at python_async_http_requests/.

## Systemtest

The Systemtest runs a Client, which uses pyahr, to do some HTTP Requests against a Server.
A Testenvironment is set up with docker-compose. Server and Client are started. 
The Client executes its Script and Terminates. This is a manual Test and it is Successfull if no Errors are logged.

