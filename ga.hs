{-
A simple genetic algorithm written in Haskell.
Written by Marshall Beddoe <mbeddoe@gmail.com>
December 2007

The purpose of this algorithm is to find the value of x which maximizes
the function f(x) = sin(pi*x/256) over the range 0 <= x <= 255, where x is
an integer.

To do:
  Create a polynomial generator that creates one of a specified degree.
  Allow crossover rate to be configurable
  Finish mutate function and make rate configurable
-}

module Main where

import Bits
import Monad
import Probability
import Text.Printf

-- The function f we want to maximize
f :: Int -> Float
f x = sin (pi * (fromIntegral x) / 256)

-- Initialize the population using random values
initialize :: Int -> R [Int]
initialize n = select n $ uniform [0..255]

-- Evaluate population
evaluate :: [Int] -> [Float]
evaluate = map f

-- Calculate fitness of each member of the population
fitness :: (Fractional a) => [a] -> [a]
fitness p = map (/total) p
    where
      total = sum p

-- Weighted random selection from population based on fitness
wheel :: [Int] -> Dist Int
wheel population = mkD $ zip population $ fitness $ evaluate population

-- Select n of the fittest members
select :: Int -> Dist Int -> R [Int]
select n dist = sequence $ take n $ repeat $ pick dist

-- One point crossover operation
crossover :: [Int] -> R [Int]
crossover [] = return []
crossover [x] = return [x]
crossover (x:y:zs) =
    pick splicePoints >>= \p ->
        let x' = (x .&. (complement (p-1))) .|. (y .&. (p-1))
            y' = (y .&. (complement (p-1))) .|. (x .&. (p-1))
        in return (x':y':[] ++ zs)
    where
      splicePoints = uniform [2^x | x <- [1..7]]

-- Random mutations against the population
mutate :: [Int] -> R [Int]
mutate x = return x

breed :: [Int] -> Int -> R [Int]
breed p 0 = return p
breed p n = select m (wheel p) >>= crossover >>= mutate >>= flip breed (n-1)
    where m = length p

avgFitness :: Fractional b => [b] -> b
avgFitness pop = (sum pop) / total
    where
      total = fromIntegral $ length pop

popSize = 32 :: Int
rounds  = 100 :: Int

main = do
  pop <- initialize popSize
  printf "inital population average fitness: %f\n" (avgFitness $ evaluate pop)
  x <- (breed pop rounds)
  printf "evolved population average fitness: %f\n" (avgFitness $ evaluate x)
  return ()
