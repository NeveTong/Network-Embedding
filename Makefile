#LINE

CC = g++
CFLAGS = -ggdb -lm -pthread -Ofast -march=native -Wall -O3 -fopenmp -Igzstream -Isrc -IHLBFGS -funroll-loops -ffast-math -Wno-unused-result
LDFLAGS = -lgomp -lgzstream -lz -lstdc++ -Lliblbfgs-1.10/lib/.libs -Lgzstream -lgsl -lm -lgslcblas
OBJECTS = objects/common.o objects/corpus.o objects/topicCorpus.o objects/reconstruct.o objects/line.o objects/concatenate.o objects/validation.o gzstream/gzstream.o

DATAPATH = /home/auburn/Documents/productGraph/data
LAMBDA = 0.5
BANDWIDTH = 1000
PRODUCTSPERTOPIC = 200
DEPTH = 2
THRESHOLD = 1000
BINARY = 1
SIZE = 128
ORDER = 2
NEGATIVE = 5
RHO = 0.025
THREADS = 40

all: train

gzstream/gzstream.o:
	cd gzstream && make

objects/common.o: src/common.hpp src/common.cpp Makefile
	$(CC) $(CFLAGS) -c src/common.cpp -o $@

objects/corpus.o: src/corpus.hpp src/corpus.cpp objects/common.o gzstream/gzstream.o Makefile
	$(CC) $(CFLAGS) -c src/corpus.cpp -o $@

objects/topicCorpus.o: src/topicCorpus.cpp src/topicCorpus.hpp objects/corpus.o objects/common.o Makefile
	$(CC) $(CFLAGS) -c src/topicCorpus.cpp -o $@

objects/reconstruct.o: src/reconstruct.cpp src/topicCorpus.hpp Makefile
	$(CC) $(CFLAGS) -c src/reconstruct.cpp -o $@

objects/line.o: src/line.cpp src/topicCorpus.hpp Makefile
	$(CC) $(CFLAGS) -c src/line.cpp -o $@

objects/concatenate.o: src/concatenate.cpp src/topicCorpus.hpp Makefile
	$(CC) $(CFLAGS) -c src/concatenate.cpp -o $@

objects/validation.o: src/validation.cpp src/topicCorpus.hpp Makefile
	$(CC) $(CFLAGS) -c src/validation.cpp -o $@

train: src/main.cpp $(OBJECTS) gzstream/gzstream.o Makefile
	$(CC) $(CFLAGS) -o train src/main.cpp $(OBJECTS) $(LDFLAGS)

clean:
	rm -rf objects/*.o gzstream/gzstream.o train

%.out: train
	./train \
	$(DATAPATH)/reviews_$*.votes.gz \
	$(LAMBDA) \
	$(BANDWIDTH) \
	$(PRODUCTSPERTOPIC) \
	$(DATAPATH)/productMeta.txt.gz \
	null \
	$(DATAPATH)/bought_together.txt.gz \
	trainFile_bought_together.txt \
	$(DEPTH) \
	$(THRESHOLD) \
	$(BINARY) \
	$(SIZE) \
	$(ORDER) \
	$(NEGATIVE) \
	$(RHO) \
	$(THREADS) | tee $@

