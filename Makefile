SOURCE_SINGLE=countChars-onlySingles.cpp
SOURCE_DOUBLE=countChars-singlesDoubles.cpp
SOURCE_COMBS=countChars-all.cpp
SINGLE=CountSingleLetters
DOUBLE=CountSingleLettersAndRepeats
COMBS=CountSingleLettersAndCombinations

CC=g++

all: $(SINGLE) $(DOUBLE) $(COMBS)

$(SINGLE):
	$(CC) $(SOURCE_SINGLE) -o $(SINGLE)

$(DOUBLE):
	$(CC) $(SOURCE_DOUBLE) -o $(DOUBLE)

$(COMBS): 
	$(CC) $(SOURCE_COMBS) -o $(COMBS)

single: $(SINGLE)

double: $(DOUBLE)

combs: $(COMBS)

clean: 
	rm -f $(SINGLE) $(DOUBLE) $(COMBS) *.o temp

