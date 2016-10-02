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

echo
echo "Testing sample Sudokus (should all be solved)"
echo

# standard examples should all work
for i in examples/* 
 do 
   if [ -f $i ]; then
   echo -n "$i: " 
   if ./out/sudoku-solver.exe  $i | grep "HAS BEEN SOLVED" >/dev/null
   then 
     echo -n "SOLVED => "
	 ok
  else
    echo -n "+++++ FAIL +++++ => "
	nok
  fi
	fi 
done

echo
echo "Testing difficult Sudokus (cannot be solved)"
echo

# too difficult
for i in examples/too_difficult/*
 do 
   if [ -f $i ]; then
   echo -n "$i: " 
   if ./out/sudoku-solver.exe  $i | grep "could not be solved" >/dev/null
   then 
     echo -n "NOT SOLVED => "
	 ok
  else
    echo -n "SOLVED? => "
	nok
  fi 
  fi
done


echo
echo "Testing invalid Sudokus (should yield an error)"
echo

# invalid
for i in examples/invalid/*
 do 
   if [ -f $i ]; then
   echo -n "$i: " 
   if ./out/sudoku-solver.exe  $i | grep "ERROR" >/dev/null
   then 
     echo -n "ERROR => "
	 ok
  else
    echo -n "NO ERROR => "
	nok
  fi 
  fi
done
