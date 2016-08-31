Wie kommts zu dieser Assertion-Verletzung:

    alex@kaktus /d/d/d/git/divers/sudokusolver
    $ ./out/sudoku-solver.exe
    Reading Sudoku from file examples/naked-triple-0.sudoku ...
    assertion "field->candidatesLeft > 0" failed: file "src/grid.c", line 649, function: removeCandidate
    Aborted (Speicherabzug geschrieben)

Debugging:
	
    (gdb) b grid.c:643 if field->candidatesLeft == 1
    Breakpoint 1 at 0x10040501a: file src/grid.c, line 643.
    
    (gdb) r
    Starting program: /cygdrive/d/d/git/divers/sudokusolver/out/sudoku-solver.exe
    [New Thread 4860.0x1060]
    [New Thread 4860.0xfc8]
    [New Thread 4860.0x998]
    [New Thread 4860.0x498]
    Reading Sudoku from file examples/naked-triple-0.sudoku ...
    
    Breakpoint 1, removeCandidate (field=0x60004a330, candidate=8) at src/grid.c:643
    643         c = field->candidates + candidate - 1;
    
    (gdb) bt
    #0  removeCandidate (field=0x60004a330, candidate=8) at src/grid.c:643
    #1  0x0000000100404a34 in forbidNumbersInOtherFields (container=0x60004c5a0, n=0x60004f210, dontTouch=0x60004ebe0) at src/grid.c:374
    #2  0x0000000100401b27 in eliminateFieldsCandidatesFromOtherFields (container=0x60004c5a0, fields=0x60004ebe0) at src/solve.c:634
    #3  0x0000000100401933 in recurseNakedTuples (maxLevel=3, container=0x60004c5a0, level=1, includedFields=0x60004ebc0, fieldsLeft=0x60004ec10) at src/solve.c:548
    #4  0x00000001004017de in findNakedTuplesInContainer (container=0x60004c5a0, dimension=3, includedFields=0x60004ebc0, fieldsLeft=0x60004ec10) at src/solve.c:473
    #5  0x00000001004015e1 in findNakedTuples () at src/solve.c:253
    #6  0x0000000100401269 in solve () at src/solve.c:87
    #7  0x0000000100402cd2 in main (argc=1, argv=0xffffcc10) at src/main.c:117
    (gdb)

Field = C7 (correct solution: 8, einziger uebruger Kandidat ist auch 8)

Der korrekte Kandidat 8 soll hier entfernt werden - warum?
Der korrekte Kandidat 8 soll hier entfernt werden - warum?

---------

So, nochmal, diesmal mit Einbau einer Assertion, wenn eine richtige Lösung als Kandidat entfernt wird.

    b grid.c:653 if field->correctSolution == candidate

    Starting program: /cygdrive/d/d/git/divers/sudokusolver/out/sudoku-solver.exe
    [New Thread 4828.0x320]
    [New Thread 4828.0x16fc]
    [New Thread 4828.0x340]
    [New Thread 4828.0x12fc]
    Reading Sudoku from file examples/naked-triple-0.sudoku ...
    
    Breakpoint 1, removeCandidate (field=0x60004a330, candidate=8) at src/grid.c:653
    653                 assert(candidate != field->correctSolution);
    (gdb) bt
	#0  removeCandidate (field=0x60004a330, candidate=8) at src/grid.c:653
	#1  0x0000000100404a34 in forbidNumbersInOtherFields (container=0x60004c5a0, n=0x60004f210, dontTouch=0x60004ebe0) at src/grid.c:374
	#2  0x0000000100401b27 in eliminateFieldsCandidatesFromOtherFields (container=0x60004c5a0, fields=0x60004ebe0) at src/solve.c:634
	#3  0x0000000100401933 in recurseNakedTuples (maxLevel=3, container=0x60004c5a0, level=1, includedFields=0x60004ebc0, fieldsLeft=0x60004ec10) at src/solve.c:548
	#4  0x00000001004017de in findNakedTuplesInContainer (container=0x60004c5a0, dimension=3, includedFields=0x60004ebc0, fieldsLeft=0x60004ec10) at src/solve.c:473
	#5  0x00000001004015e1 in findNakedTuples () at src/solve.c:253
	#6  0x0000000100401269 in solve () at src/solve.c:87
	#7  0x0000000100402cd2 in main (argc=1, argv=0xffffcc10) at src/main.c:117
    (gdb)

Field = C7 (correct solution: 8, einziger übriger Kandidat ist auch 8)

Der korrekte Kandidat 8 soll hier entfernt werden - warum?
Der korrekte Kandidat 8 soll hier entfernt werden - warum?

Interessant: 


    634         progress = forbidNumbersInOtherFields(container, candidates, fields);
Hier wird (2, 7, 8) verboten in Row C, außer Felder C3, C6 und C6 (sic!). Warum 2-mal C6?

=> Spezieller Unit-Test test_findNakedTriplesInContainer2 dafür erstellt.




Pointing Pairs
==============

Sudoku:
000724005020010070000080002090036250602070008053240010400390020030062090209457000

Solution:
981724365324615879765983142197836254642571938853249716476398521538162497219457683

	(gdb) b grid.c:658 if candidate == field->correctSolution
	Breakpoint 3 at 0x100405379: file src/grid.c, line 658.
	(gdb) r
	The program being debugged has been started already.
	Start it from the beginning? (y or n) y
	Starting program: /cygdrive/d/d/git/divers/sudokusolver/out/sudoku-solver.exe
	[New Thread 4424.0xb5c]
	[New Thread 4424.0x9b4]
	[New Thread 4424.0x141c]
	[New Thread 4424.0x698]
	Reading Sudoku from file examples/pointing-pair.sudoku ...
	[pii] starting findPointingTuples ...

	Breakpoint 3, removeCandidate (field=0x60004a138, candidate=8) at src/grid.c:658
	658                 assert(candidate != field->correctSolution);
	(gdb) p field->name
	$7 = "B7\000"
	(gdb)
	[0] 0:gdb* 1:bash-                                                 

Er möchte in G9 Kandidat 1 entfernen - das ist aber die richtige Lösung! Warum?

Backtrace dazu:

	#0  removeCandidate (field=0x60004ab80, candidate=1) at src/grid.c:658
	#1  0x0000000100404d83 in forbidNumbersInOtherFields (container=0x60004c7e0, n=0x60004ec20, dontTouch=0x60004ebc0) at src/grid.c:379
	#2  0x0000000100401d51 in eliminateCandidatesFromOtherFields (container=0x60004c7e0, fields=0x60004ebc0, candidate=1) at src/solve.c:669
	#3  0x0000000100401b18 in findPointingTuplesInContainer (container=0x60004c6f0, fieldsWithCandidate=0x60004ebc0) at src/solve.c:564
	#4  0x0000000100401a2b in findPointingTuples () at src/solve.c:491
	#5  0x000000010040127f in solve () at src/solve.c:92
	#6  0x0000000100403012 in main (argc=1, argv=0xffffcc10) at src/main.c:117

	
	
Hidden Tuples
==========

Input: Sudoku "hidden-tuples.sudoku"

Segfault in Rekursionstiefe 2 bei:

	Program received signal SIGSEGV, Segmentation fault.
	0x000000010040461b in countDistinctFields (fields=0x60005e9d0, candidates=0x60005f100, limit=2, fieldsWithCandidates=0x60005f150) at src/hidden-tuples.c:344
	(gdb) bt
	#0  0x000000010040461b in countDistinctFields (fields=0x60005e9d0, candidates=0x60005f100, limit=2, fieldsWithCandidates=0x60005f150) at src/hidden-tuples.c:344
	#1  0x0000000100404344 in recurseHiddenTuples (maxLevel=2, container=0x60005dc40, level=2, includedCandidates=0x60005ea20, candidatesLeft=0x60005f124,
		fieldsWithCandidates=0x60005f150) at src/hidden-tuples.c:166
	#2  0x0000000100404386 in recurseHiddenTuples (maxLevel=2, container=0x60005dc40, level=1, includedCandidates=0x60005ea20, candidatesLeft=0x60005f120,
		fieldsWithCandidates=0x60005f150) at src/hidden-tuples.c:170
	#3  0x0000000100404236 in findHiddenTuplesInContainer (container=0x60005dc40, dimension=2, includedCandidates=0x60005ea20, candidatesLeft=0x60005f120,
		fieldsWithCandidates=0x60005f150) at src/hidden-tuples.c:106
	#4  0x000000010040b735 in test_findHiddenPairInContainer () at test/testGridFunctions.c:676
	#5  0x000000010040325a in UnityDefaultTestRun (Func=0x10040b460 <test_findHiddenPairInContainer>, FuncName=0x10040fc80 <__func__.3914+736> "test_findHiddenPairInContainer",
		FuncLineNum=709) at ../Unity/src/unity.c:1284
	#6  0x000000010040bb1e in main () at test/testGridFunctions.c:709
	(gdb)


	
X-Sudoku
========
Input: Sudoku "x-sudoku.standard.3454b"
070020001001603840008000200000000700503090104007000000006000500032905400700040020



