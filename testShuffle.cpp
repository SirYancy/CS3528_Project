#include <random>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>

int main() {
  std::vector<int> mixedPopulation;
  mixedPopulation.resize(20);
  
  std::vector<int> newPop1, newPop2, newPop3, newPop4;
  
  std::iota(mixedPopulation.begin(), mixedPopulation.end(), 0);
  
  std::cout << "Before mixed: " << std::endl;

  for (auto iter = mixedPopulation.begin(); iter != mixedPopulation.end(); ++iter) {
    std::cout << *iter << " ";
  }
  
  std::cout << std::endl;
  
  
    // Seed nice random number generator.
    std::mt19937 rng(std::random_device{}());

    // Shuffle up the indices.
    std::shuffle(mixedPopulation.begin(), mixedPopulation.end(), rng);

  std::cout << "After mixed: " << std::endl;

  for (auto iter = mixedPopulation.begin(); iter != mixedPopulation.end(); ++iter) {
    std::cout << *iter << " ";
  }
  
  std::cout << std::endl;
  
    while(mixedPopulation.size() > 0) {
        if (mixedPopulation.size() > 0) {
            newPop1.push_back(mixedPopulation.back());
            //std::cout << &mixedPopulation.back() << std::endl;
            mixedPopulation.pop_back();
        }
        if (mixedPopulation.size() > 0) {
            newPop2.push_back(mixedPopulation.back());
            //std::cout << &mixedPopulation.back() << std::endl;
            mixedPopulation.pop_back();
        }
        if (mixedPopulation.size() > 0) {
            newPop3.push_back(mixedPopulation.back());
            //std::cout << &mixedPopulation.back() << std::endl;
            mixedPopulation.pop_back();
        }

        if (mixedPopulation.size() > 0) {
            newPop4.push_back(mixedPopulation.back());
            //std::cout << &mixedPopulation.back() << std::endl;
            mixedPopulation.pop_back();
        }
    }

  std::cout << "Pop1: " << std::endl;

  for (auto iter = newPop1.begin(); iter != newPop1.end(); ++iter) {
    std::cout << *iter << " ";
  }
  std::cout << std::endl;
  std::cout << "Pop2: " << std::endl;

  for (auto iter = newPop2.begin(); iter != newPop2.end(); ++iter) {
    std::cout << *iter << " ";
  }
  std::cout << std::endl;
  std::cout << "Pop3: " << std::endl;

  for (auto iter = newPop3.begin(); iter != newPop3.end(); ++iter) {
    std::cout << *iter << " ";
  }
  std::cout << std::endl;
  std::cout << "Pop4: " << std::endl;
  for (auto iter = newPop4.begin(); iter != newPop4.end(); ++iter) {
    std::cout << *iter << " ";
  }

    
    return 0;
}
