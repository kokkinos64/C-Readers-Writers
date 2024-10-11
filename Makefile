default:
# MallocCheck:
	gcc -g3 -c MallocCheck.c -lpthread
# SharedStruct
	gcc -g3 -c SharedStruct.c -lpthread
# Reader:
	gcc -g3 -c reader.c -lpthread
	gcc -g3 -o reader reader.o MallocCheck.o SharedStruct.o -lpthread
# write:
	gcc -g3 -c write.c
	gcc -g3 -o write write.o MallocCheck.o SharedStruct.o -lpthread
# Allocator
	gcc -g3 -c allocator.c
	gcc -g3 -o allocator allocator.o SharedStruct.o MallocCheck.o -lpthread
# Dislocator
	gcc -g3 -o dislocator dislocator.c -lpthread
# Conductor
	# gcc -g3 -c conductor.c
	# gcc -g3 -o conductor conductor.o MallocCheck.o -lpthread
# createReaders
	gcc -g3 -c createReaders.c
	gcc -g3 -o createReaders createReaders.o MallocCheck.o -lpthread
# createWriters
	gcc -g3 -c createWriters.c
	gcc -g3 -o createWriters createWriters.o MallocCheck.o -lpthread
# Print Stats
	gcc -g3 -c monitor.c -lpthread
	gcc -g3 -o monitor monitor.o SharedStruct.o MallocCheck.o -lpthread
# Remove object files
	rm -f *.o

# Remove semaphores from system
removesem:
	rm /dev/shm/sem.prj3_1
	rm /dev/shm/sem.prj3_2
	rm /dev/shm/sem.prj3_3