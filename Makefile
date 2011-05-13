all:
	ghc -O -c ListUtils.hs
	ghc -O -c Show.hs
	ghc -O -c Probability.hs
	ghc -O -c ga.hs
	ghc -O -o ga *.o
clean:
	rm -f *.hi *.o ga
