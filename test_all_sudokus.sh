#!/bin/bash


function green {
	echo -n -e "\033[0;32m"
}

function red {
	echo -n -e "\033[0;31m"
}

function normal {
	echo -n -e "\033[0m"
}

function ok {
	green; echo "PASS"; normal
}

function nok {
	red; echo "FAIL"; normal
}

# standard examples should all work
for i in examples/* 
 do 
   echo -n "$i: " 
   if ./out/sudoku-solver.exe  $i | grep "HAS BEEN SOLVED" >/dev/null
   then 
     echo -n "SOLVED => "
	 ok
  else
    echo -n "+++++ FAIL +++++ => "
	nok
  fi 
done

# too difficult
for i in examples/too_difficult 
 do 
   echo -n "$i: " 
   if ./out/sudoku-solver.exe  $i | grep "could not be solved" >/dev/null
   then 
     echo -n "NOT SOLVED => "
	 ok
  else
    echo -n "SOLVED? => "
	nok
  fi 
done
