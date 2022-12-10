@echo off
rem eg.: .\setpal.cmd espbadoben1 0 255 0 0 0
curl -H "Content-Type: application/json" -X POST http://%1/api -d "{ \"settings\": { \"setpal\": { \"idx\": %2, \"r\": %3, \"g\": %4, \"b\": %5 } } }"
@echo on