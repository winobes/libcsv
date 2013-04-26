lib_dir = /usr/local/lib/
include_dir = /usr/local/include/

libcsv.a : csv.o
	ar -cvq libcsv.a csv.o

csv.o : src/csv.c src/csv.h
	gcc -Wall -c src/csv.c

$(lib_dir)libcsv.a : libcsv.a 
	mv libcsv.a $(lib_dir) 

$(include_dir)csv.h : include/csv.h
	cp include/csv.h $(include_dir)

install : $(lib_dir)libcsv.a $(include_dir)csv.h

.PHONY : uninstall
uninstall : 
	rm -f $(lib_dir)libcsv.a
	
.PHONY : clean
clean :
	rm -f csv.o libcsv.a
