gdb --args lfileenc remote.png rem.png
this will configure the args for lfileenc and
to running it only with r will suffice

set logging on 		# log output to gdb.txt
set logging off 	# disable logging

step 6  			# step into 6 lines ahead
step 2				# step 2 lines ahead

disassemble /m b32_enc # disassemble b32_enc and interleave the C code with the
						 assembly, I think it's not exactly precise but it's a
						 clue of C vs ASM equivalence


to find most abbreviations of command test them with help, example:
   's' help  will print the help for the 'step' command
