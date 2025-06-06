chess: chess.cpp *.h
	g++ -o chess chess.cpp -std=c++20 -Ofast

clean:
	rm -f chess
