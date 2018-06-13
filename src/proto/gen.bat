@echo off
protoc -I=. --cpp_out=. ./message.proto
pause