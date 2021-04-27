#!/bin/bash
touch run_bmi
mv -f run_bmi z_trash
gcc -lm ./src/et.c ./src/bmi_et.c -o run_bmi
./run_bmi ./configs/et_config_unit_test1.txt 
./run_bmi ./configs/et_config_unit_test2.txt 
./run_bmi ./configs/et_config_unit_test3.txt 
./run_bmi ./configs/et_config_unit_test4.txt 
./run_bmi ./configs/et_config_unit_test5.txt 
