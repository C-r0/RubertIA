nasm -f elf64 parser.asm -o build/parser.o

g++ -c brain.cpp -o build/brain.o

g++ build/parser.o build/brain.o -o build/RubertIA -lcurl -no-pie
