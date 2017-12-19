@echo off
echo *** Generating data file MVT.txt
if exist MVT.dat del MVT.dat 
..\Gendata -A MVT.txt


echo *** Generating data file EST.txt
if exist EST.dat del EST.dat 
..\Gendata -A EST.txt


