[windows]
build:
  g++ \
    -g -Wall -static \
    -o main.exe \
    -I include/ \
    include/railroad/*.cpp \
    src/*.cpp \
    -lreadline \
    -lncurses

[windows]
run: build
  ./main.exe

docs:
  pandoc --toc -H docs/Relatório/disable_float.tex --resource-path="docs/Relatório" docs/Relatório/Relatório.md -o Relatório.pdf