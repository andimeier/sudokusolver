#!/bin/bash

 for i in examples/* 
 do 
   echo -n "$i: " 
   if ./out/sudoku-solver.exe  $i | grep "HAS BEEN SOLVED" >/dev/null
   then 
     echo "SOLVED"
  else
    echo "+++++ FAIL +++++"
  fi 
done
