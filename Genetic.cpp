#include "Genetic.h"
//#include "Client.h"
//#include "Package.h"
#include <vector>
#include <iostream>
#include <cmath>
//#include <thread>
#include <future>
#include <random>

// Seed nice random number generator.
std::mt19937 rngGenetic(std::random_device{}());

Genetic::Genetic(std::vector<Package*> packs,
                 std::vector<vector<unsigned int> > matrix,
                 unsigned int weight,
                 unsigned int packLimit,
                 unsigned int population,
                 float stops, float drive, float shiftTime, unsigned long gens, mutation_enum mut)
                 : packages(packs), adMatrix(matrix), weightLimit(weight), packageLimit(packLimit),
                 stopTime(stops), driveTime(drive), timeLimit(shiftTime), generations(gens), mutation(mut)

{
    Priority currentPriority;

    // Truncate population to multiples of four for threading.
    //popNum = (population / 4) * 4;
    popNum = population;

    std::cout << "Population: " << popNum << " Generations: " << gens << std::endl;

    // Total for roulette selection of mutation.
    //mutation.total = mutation.crossOver + mutation.deleteOld + mutation.insertNew + mutation.inversion + mutation.swapOut + mutation.swapWithin;
    mutation.total = mutation.deleteOld + mutation.insertNew + mutation.inversion + mutation.swapOut + mutation.swapWithin;

    // Create the best fitness info vector for later indexing.
    for (unsigned int i = 0; i < 5; ++i) {
        bestFitInfo.push_back(0);
    }

    // How many packages are there in the pool.
    numOfPackages = packages.size();

    // How many good members will be carried over into the next population
    elitist = mutation.elite * popNum;

    for (vector<Package* >::iterator iter = packages.begin(); iter != packages.end(); ++iter) {
        currentPriority = (*iter)->getPriority();

        // Assign weight.
        if (currentPriority == Priority::OVERNIGHT) {
            totalPriority += OVERNIGHT_WEIGHT;
        } else if (currentPriority == Priority::TWO_DAY) {
            totalPriority += TWODAY_WEIGHT;
        } else {
            totalPriority += REGULAR_WEIGHT;
        }
    }
}

Genetic::~Genetic()
{
    //dtor
}

size_t Genetic::hash(vector<Package* >* gene) const {
    size_t seed = gene->size();
    for (auto iter = gene->begin(); iter != gene->end(); ++iter) {
        seed ^= (*iter)->getID() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    return seed;
}
void Genetic::initPopulation() {
    // Package distribution
    std::uniform_int_distribution<int> packageUniform(0, numOfPackages - 1);

    std::vector<Package* > individual;

    std::vector<double> returnedFitness;

    genes.resize(popNum);

    float totalWeight = 0;
    unsigned int totalPackages = 0;
    unsigned int maxTries = 1000;
    unsigned int tries = 0;


    Package* currentPackage;
    unsigned int randomPackage;
    bool present = false;

    for (unsigned int i = 0; i < popNum; i++) {
        //std::cout << "Individual " << i << std::endl;

        // Clear individual
        individual.clear();
        totalWeight = 0;
        totalPackages = 0;
        tries = 0;

        geneInfo currentGeneInfo;

        // Loop while we still have packages to place, and we're under the limits.
        while (totalPackages < numOfPackages) { // && totalPackages < packageLimit&& totalWeight < weightLimit && (totalPackages * stopTime) < (timeLimit / 2) && tries < maxTries) {

            tries++;

            // Random index to packages
            randomPackage = packageUniform(rngGenetic);

            //std::cout << "Random package: " << to_string(randomPackage << std::endl;
            // Get the package pointer
            currentPackage = packages[randomPackage];

            //std::cout << "Package ptr: " << std::to_string(currentPackage->getID()) << std::endl;
            // Assume not present in current individual
            present = false;

            // Loop over the individual looking for duplicate package
            for (std::vector<Package*>::iterator iter = individual.begin(); iter != individual.end(); ++iter) {
                // Check duplicate
                if (*iter == currentPackage) {
                    present = true;
                    //std::cout << "Package already present: " << std::to_string((*iter)->getID()) << std::endl;
                    break;
                }
            }

            // If not a duplicate
            if (present == false) {
                // Over truck limit if added?
                //if (currentPackage->getWeight() + totalWeight <= weightLimit) {
                    // Add package to individual
                    individual.push_back(currentPackage);
                    //std::cout << "Adding package ptr: " << currentPackage << std::endl;
                    totalPackages++;
                    totalWeight += currentPackage->getWeight();

                //}

            }
        }

        //std::cout << "Current weight: " << totalWeight << endl;
        //genes.push_back(std::make_pair(individual, 0));
        currentGeneInfo.fitnessValue = fitness(&individual);
        currentGeneInfo.hashValue = hash(&individual);
        currentGeneInfo.sizeValue = individual.size();

        genes[i] = std::make_pair(individual, currentGeneInfo);


        //std::cout << "Adding package of length: " << individual.size() << std::endl;
        //printGene(&individual);
        //std::cout << std::endl;
        //std::cout << "Hash: " << hash(&individual) << std::endl;


    }


    initRanking(4);

}

void Genetic::initRanking(float exponent) {
    // Resize the ranking vector to evolution.
    ranking.resize(genes.size());
    rankingSize = ranking.size();

    // Load exponential ranking array. Always the same number to individuals, so same ranking
    for (unsigned int i = 0; i < rankingSize; ++i) {
        // Exponential ranking
        ranking[i] = pow(i+1, exponent);
        //std::cout << "Ranking[" << i << "]=" << ranking[i] << std::endl;
        // Add rank to total
        rankTotal += ranking[i];
    }
}

vector<double> Genetic::fitness(vector<Package* >* individual) {
    unsigned int distance = 0;
    unsigned int shiftTime = 0;
    unsigned int priorities = 0;
    Priority currentPriority;

    float weight = 0;

    unsigned int previousIndex = 0, currentIndex = 0;

    float indFit = 0;

    previousIndex = 0;

    for (std::vector<Package*>::iterator iter = (*individual).begin(); iter != (*individual).end(); ++iter) {
        currentIndex = (*iter)->getReceiver()->getID();
        //std::cout << " Current Index: " << currentIndex << std::endl;
        //if (currentIndex >= numOfPackages) {
            //std::cout << "Number of packages: " << numOfPackages << " Current Index: " << currentIndex << std::endl;
        //}
        distance += adMatrix[previousIndex][currentIndex];

        //std::cout << "Distance[" << previousIndex << "][" << currentIndex << "] = " << distance << std::endl;

        previousIndex = currentIndex;
        currentPriority = (*iter)->getPriority();

        // Assign weight.
        if (currentPriority == Priority::OVERNIGHT) {
            priorities += OVERNIGHT_WEIGHT;
        } else if (currentPriority == Priority::TWO_DAY) {
            priorities += TWODAY_WEIGHT;
        } else {
            priorities += REGULAR_WEIGHT;
        }

        weight += (*iter)->getWeight();

        //std::cout << "Package priority: " << static_cast<int>((*iter)->getPriority()) << std::endl;
        shiftTime += stopTime;
    }

    distance += adMatrix[previousIndex][0];



    shiftTime += distance * driveTime;

    if (shiftTime > timeLimit) {
        indFit -= (pow(shiftTime - timeLimit, 1.5) * priorities);
    } else {
        //indFit += pow(static_cast<float>(timeLimit) - static_cast<float>(shiftTime), 1.15);
        //indFit += pow(priorities, 1.0 + ((static_cast<float>(timeLimit) - static_cast<float>(shiftTime)) / static_cast<float>(shiftTime)));
    }

    //std::cout << 2.0 + ((static_cast<float>(timeLimit) - static_cast<float>(shiftTime)) / static_cast<float>(shiftTime)) << std::endl;
    //indFit += pow((static_cast<float>(OVERNIGHT) * static_cast<float>(individual.size()))/static_cast<float>(priorities), 1.5);
    indFit += pow(static_cast<float>(priorities), 1.5);


    if (weight > weightLimit) {
        indFit -= pow(weight - weightLimit, 1.5) * priorities;
    }

    if (individual->size() > packageLimit) {
        indFit -= (pow(individual->size() - packageLimit, 1.5) * priorities);
    }
    //indFit = pow(timeLimit/static_cast<float>(shiftTime),2) + (static_cast<float>(OVERNIGHT) * static_cast<float>(individual.size()))/static_cast<float>(priorities);// + pow(static_cast<float>(weightLimit - weight), 2);

    //std::cout << "Individual length: " << individual->size() << " Distance: " << distance << "Fit: " << indFit << std::endl;
    //std::cout << "F: " << indFit << "D: " << distance << " T: " << shiftTime << " Pri: " << priorities << " Weight: " << weight << " Len: " << individual->size() << std::endl;
    //std::cout.precision(17);
    //std::cout << 1/static_cast<float>(shiftTime) << std::endl;
    //std::cout << "Fit: " << indFit << std::endl;
    //std::cout << "OVERNIGHT: " << static_cast<int>(OVERNIGHT) << std::endl;
    vector<double> fit {indFit, priorities, distance, shiftTime, weight};
    return fit;
}

void Genetic::loadPopulation(vector< pair<vector<Package* >, geneInfo> > newPopulation) {

    genes = newPopulation;
    popNum = newPopulation.size();

    // Initialize current fitness.
    vector<float> currentFitness (5, 0);

    initRanking(2);

/*
    for (auto iter = genes.begin(); iter != genes.end(); ++iter) {
        currentFitness = fitness((*iter).first);
        (*iter).second = currentFitness[0];
    }
*/

    mergeSort(0, genes.size() - 1);
/*
    for (auto iter = genes.begin(); iter != genes.end(); ++iter) {
        for (auto jitter = iter->first.begin(); jitter != iter->first.end(); ++jitter) {
            std::cout << (*jitter)->getID() << " ";
        }
        std::cout << "   <- " << iter->second.fitnessValue << std::endl;
    }
*/
    return;

}

vector< pair<vector<Package* >, Genetic::geneInfo> > Genetic::evolve_threads() {
    evolve();
    return genes;

}

void Genetic::printGene(vector<Package* >* gene) const {
       for (auto iter = gene->begin(); iter != gene->end(); ++iter) {
            std::cout << (*iter)->getID() << " ";
       }

       //std::cout << std::endl;
}

vector<Package* > Genetic::evolve() {

    //initPopulation();
    vector<double> currentFitness (5, 0);
    vector<double> currentBest (5, 0);

    int highestID = 0;

    std::vector<std::pair<std::vector<Package* >, geneInfo> >::iterator row;

    for (unsigned long i = 0; i < generations; ++i) {
            currentBest[0] = 0;
        for (row = genes.begin(); row != genes.end(); ++row) {
            /*
            for (auto currentGene = (*row).first.begin(); currentGene != (*row).first.end(); ++currentGene) {
                if ((*currentGene)->getID() > highestID) {
                    highestID = (*currentGene)->getID();
                }
            }
            */
            //std::cout << "Highest ID: " << highestID << std::endl;

                //std::cout << "Current iter ptr: " << (*row) << std::endl;
                //std::cout << "Before fitness: " << (*row).first.size() << std::endl;

                currentFitness = row->second.fitnessValue; //fitness(&(*row).first);
                //(*row).second.fitnessValue = currentFitness[0];
                if (currentFitness[0] > currentBest[0] || currentBest[0] == 0) {
                    currentBest = currentFitness;
                }

                if (currentFitness[0] > bestFitInfo[0] || bestFitInfo[0] == 0) {
                    bestFit = (*row).first;
                    bestFitInfo = currentFitness;
                }
                //std::cout << "Current fitness: " << currentFitness[0] << std::endl;
        }

        if (i % 1000 == 0) {
            std::cout << "Generation " << i << " Best F: " << bestFitInfo[0] << " P: " << bestFitInfo[1] << "/" << totalPriority << " D: " << bestFitInfo[2] << " T: " << bestFitInfo[3] << "/" << timeLimit << " W: " << bestFitInfo[4] << "/" << weightLimit << " L: " << bestFit.size() << "/" << numOfPackages << " CO: " << avgIndividual << std::endl;//" F: " << genes[popNum - elitist - 2].second << " P: " << currentBest[1] << " D: " << currentBest[2] << " T: " << currentBest[3] << "/" << timeLimit << " W: " << currentBest[4] << "/" << weightLimit << std::endl;
        }
        mate();
    }
    std::cout << "Best  F: " << bestFitInfo[0] << " P: " << bestFitInfo[1] << "/" << totalPriority << " D: " << bestFitInfo[2] << " T: " << bestFitInfo[3] << "/" << timeLimit << " W: " << bestFitInfo[4] << "/" << weightLimit << " L: " << bestFit.size() << std::endl;

    /*
    mergeSort(0, genes.size() - 1);

    for (auto iter = genes.begin(); iter != genes.end(); ++iter) {
        //printGene(&(*iter).first);
        std::cout << iter->second.fitnessValue[0] << " Hash: " << iter->second.hashValue << std::endl;
    }
*/
    return getBest();
}

pair<int, int> Genetic::chooseParents() {

    vector<vector<Package* > > newIndividuals;
    newIndividuals.resize(2);

    std::uniform_real_distribution<double> rankUniform(0, rankTotal);


    unsigned int smallestLength;
    unsigned int longestLength;
    unsigned int randomPoint;
    unsigned int whichFirst;
    unsigned int randomNewGene;
    unsigned int tries = 0;


    int randomIndividual1 = 0;
    int randomIndividual2 = 0;
    double rank1 = 0;
    double rank2 = 0;

    float randomP;
    unsigned int randomSwap1, randomSwap2;
    bool different = false;


    // Loop until we get different individuals
    while (different == false) {
        randomIndividual1 = 0;
        randomIndividual2 = 0;

        rank1 = rankUniform(rngGenetic); //rand() % rankTotal;
        rank2 = rankUniform(rngGenetic); //rand() % rankTotal;


        for (unsigned int i = 0; i < rankingSize; ++i) {
            //std::cout << "In1: " << rank1 << "/" << ranking[i] << " " << i << std::endl;
            if (rank1 < ranking[i]) {
                randomIndividual1 = i;
                avgIndividual = avgIndividual * 100 + static_cast<float>(i);
                //std::cout << "Individual 1: " << i << std::endl;
                // Done searching.
                break;
            } else {
                rank1 -= ranking[i];
            }

        }

        for (unsigned int i = 0; i < rankingSize; ++i) {
            //std::cout << "In2: " << rank2 << "/" << ranking[i] << " " << i << std::endl;
            if (rank2 < ranking[i]) {
                randomIndividual2 = i;
                avgIndividual = (avgIndividual + static_cast<float>(i)) / 102;
                //std::cout << "Individual 2: " << randomIndividual2 << std::endl;

                // Done searching.
                break;
            } else {
                rank2 -= ranking[i];
            }

        }

        different = false;

        //std::cout << "Gene hash 1: " << genes[randomIndividual1].second.hashValue << " Gene hash 2: " << genes[randomIndividual2].second.hashValue << std::endl;
        if (genes[randomIndividual1].second.hashValue != genes[randomIndividual2].second.hashValue) {

            different = true;

        }

            /*if (different == false) {
                std::cout << "Identical genes!!!" << std::endl;
                std::cout << "ID1: ";
                printGene(genes[randomIndividual1].first);

                std::cout << "ID2: ";
                printGene(genes[randomIndividual2].first);

            }*/

    }

    return make_pair(randomIndividual1, randomIndividual2);

}

void Genetic::mate() {
    //vector<unsigned int> ranking (genes.size(), 0);

    vector< pair<vector<Package* >, geneInfo> > newPopulation;
    vector< vector<Package* > > newIndividuals;
    pair<int, int> choosenParents;
    vector<Package* >* choosenGene;
    geneInfo currentGeneInfo1, currentGeneInfo2;

    // Resize to correct size to increase speed.
    newPopulation.resize(popNum);

    // Sort "in-place" based on fitness value. Least fit routes first in the vector, most fit last.
    mergeSort(0, popNum - 1);

    bool different = false;
    unsigned int shortest;
    //std::cout << "Before elite: " << genes.size();

    for (unsigned int i = 0; i < static_cast<int>(mutation.crossOver * popNum) / 2; ++i) {
        different = false;

        while (different == false) {
            choosenParents.first = 0;
            choosenParents.second = 0;

            while (choosenParents.first == choosenParents.second) {
                choosenParents = chooseParents();
            }
            //gene1 = (genes.at(choosenParents.first).first);
            //gene2 = (genes.at(choosenParents.second).first);
            //std::cout << "Before cross-over: [" << choosenParents.first << "," << choosenParents.second << "] = " << genes.at(choosenParents.first).first.size() << " " << genes.at(choosenParents.second).first.size() << std::endl;
/*
            std::cout << "Gene 1 [" << choosenParents.first << "]: ";
            printGene(&genes.at(choosenParents.first).first);
            std::cout << std::endl;

            std::cout << "Gene 2 [" << choosenParents.second << "]: ";
            printGene(&genes.at(choosenParents.second).first);

            std::cout << std::endl;
*/
            newIndividuals = crossOver(&genes.at(choosenParents.first).first, &genes.at(choosenParents.second).first);

            currentGeneInfo1.sizeValue = newIndividuals[0].size();
            currentGeneInfo2.sizeValue = newIndividuals[1].size();
            currentGeneInfo1.hashValue = hash(&newIndividuals[0]);
            currentGeneInfo2.hashValue = hash(&newIndividuals[1]);

            if (currentGeneInfo1.hashValue != currentGeneInfo2.hashValue) {
                different = true;
                currentGeneInfo1.fitnessValue = fitness(&newIndividuals[0]);
                currentGeneInfo2.fitnessValue = fitness(&newIndividuals[1]);
            }


            //std::cout << "Gene hash 1: " << currentGeneInfo1.hashValue << std::endl << "Gene hash 2: " << currentGeneInfo2.hashValue << std::endl;


            if (different == false) {
                std::cout << "Crossover identical" << std::endl;
            }
        }

        newPopulation[2 * i] = make_pair(newIndividuals[0], currentGeneInfo1);

        newPopulation[2 * i + 1] = make_pair(newIndividuals[1], currentGeneInfo2);

        //std::cout << "After cross-over: " << newIndividuals[0].size() << " " << newIndividuals[1].size() << std::endl;
        /*std::cout << "Gene1: ";
        printGene(newIndividuals[0]);
        std::cout << "Gene2: ";
        printGene(newIndividuals[1]);*/
    }

    for (unsigned int i = static_cast<int>(mutation.crossOver * popNum) / 2; i < static_cast<int>(popNum - elitist); ++i) {
        newPopulation[i] = genes[i];
    }

    // Create a new population not including save elite individuals.
    for (unsigned int i = 0; i < static_cast<int>(popNum - elitist); ++i) {

        choosenGene = &newPopulation[i].first;

        different = false;

        while (different == false) {
            mutate(choosenGene);

            currentGeneInfo1.hashValue = hash(choosenGene);

            different = true;

            for (auto iter = newPopulation.begin(); iter != newPopulation.end(); ++iter) {
                if (currentGeneInfo1.hashValue == (*iter).second.hashValue) {
                    different = false;
                    break;
                }
            }

            if (different == true) {
                for (auto iter = genes.begin(); iter != genes.end(); ++iter) {
                    if (currentGeneInfo1.hashValue == (*iter).second.hashValue) {
                        different = false;
                        break;
                    }
                }
            }

            //std::cout << "Mutation equal hash value!" << std::endl;
        }

        currentGeneInfo1.sizeValue = choosenGene->size();
        currentGeneInfo1.fitnessValue = fitness(choosenGene);

        newPopulation[i] = make_pair(*choosenGene, currentGeneInfo1);

    }


    // Save the elite few, the fitest. Save the Queen!

    for (unsigned int i = (popNum - elitist - 1); i < popNum; ++i) {
            //std::cout << "Elitist: " << i << std::endl;
            newPopulation[i] = genes[i];
            //std::cout << "Transferred Elitist genes: ";
            //printGene(newPopulation[i].first);
            //std::cout << std::endl;
    }


    genes = newPopulation;
    //std::cout << " After elite: " << genes.size() << std::endl;
}

vector<vector<Package*> > Genetic::crossOver(vector<Package* >* gene1, vector<Package* >* gene2) {



    // Do we have genes of size 1?
    if (gene1->size() <= 1 || gene2->size() <= 1) {
        return vector<vector<Package* > > {*gene1, *gene2};
    }

    // Size of genes.
    unsigned int geneLength1 = gene1->size();
    unsigned int geneLength2 = gene2->size();

    // Lengths of genes
    unsigned int smallestLength, longestLength;

    // Random crossover point.
    unsigned int randomPoint;

    // Gene present in both genomes flag.
    bool present;

    // New genes for the individuals
    vector<Package* > packageGene1;
    vector<Package* > packageGene2;

    // Holders return individuals.
    vector<vector<Package* > > newIndividuals;

    // Resize to save on reallocation
    newIndividuals.resize(2);


    // Which gene is longer? Could be simplified.
    if (geneLength1 < geneLength2) {
        smallestLength = geneLength1;

        longestLength = geneLength2;

    } else if (geneLength2 < geneLength1) {
        smallestLength = geneLength2;

        longestLength = geneLength1;


    } else {
        smallestLength = geneLength1;
        longestLength = geneLength1;

    }

    std::uniform_int_distribution<int> pointUniform(1, smallestLength - 1);
    // Random point to apply crossover. Don't want to cross over at 0
    randomPoint = pointUniform(rngGenetic); //(rand() % (smallestLength - 1)) + 1;



    //std::cout << std::endl << "Random point: " << randomPoint << std::endl;
    //std::cout << "G1 size: " << geneLength1 << " G2 size: " << geneLength2 << std::endl;

    // Genes crosses over from 0 to randomPoint in genes
    for (unsigned int i = 0; i <= randomPoint; ++i) {
        packageGene1.push_back((*gene2)[i]);
        packageGene2.push_back((*gene1)[i]);
    }

    // Fill individual[0] genes based on the size of gene1 in the order they appear
    for (vector<Package* >::iterator gene1_iter = gene1->begin(); gene1_iter != gene1->end(); ++gene1_iter) {

        // Assume no duplicates
        present = false;

        // Loop through all genes already placed
        for (vector<Package* >::iterator package1_iter = packageGene1.begin(); package1_iter != packageGene1.end(); ++package1_iter) {

            // Is potential gene a duplicate?
            if (*package1_iter == *gene1_iter) {
                //std::cout << "Gene already present " << gene2[i] << std::endl;
                present = true;
                break;
            }
        }

        // If not a duplicate, insert.
        if (present == false) {
            packageGene1.push_back(*gene1_iter);
            //std::cout << "Adding G1: " << (*gene1_iter)->getPointer() << std::endl;
        }
    }

    // Fill individual[0] genes based on the size of gene1 in the order they appear
    for (vector<Package* >::iterator gene2_iter = gene2->begin(); gene2_iter != gene2->end(); ++gene2_iter) {
        //std::cout << "Size: " << newIndividual.size() - 1 << std::endl;

        // Assume no duplicates
        present = false;

        // Loop through all genes already placed
        for (vector<Package* >::iterator package2_iter = packageGene2.begin(); package2_iter != packageGene2.end(); ++package2_iter) {

            //std::cout << i << " " << j << " Ind: " << newIndividual[j] << " Gene: " << gene2[i] << std::endl;

            // Is potential gene a duplicate?
            if (*package2_iter == *gene2_iter) {
                //std::cout << "Gene already present " << gene2[i] << std::endl;
                present = true;
                break;
            }
        }

        // If not a duplicate, insert.
        if (present == false) {
            packageGene2.push_back(*gene2_iter);
            //std::cout << "Adding G2: " << (*gene2_iter)->getPointer() << std::endl;
        }
    }

/*
    std::cout << "Baby crossover 1: " << std::endl;
    for(vector<Package* >::iterator iter=packageGene1.begin(); iter != packageGene1.end(); ++iter) {
        std::cout << (*iter)->getPointer() << std::endl;
    }

    std::cout << "Baby crossover 2: " << std::endl;
    for(vector<Package* >::iterator iter=packageGene2.begin(); iter != packageGene2.end(); ++iter) {
        std::cout << (*iter)->getPointer() << std::endl;
    }
*/

    newIndividuals[0] = packageGene1;
    newIndividuals[1] = packageGene2;

    //std::cout << "Returning " << &newIndividuals << std::endl;
    /*
    std::cout << "Returning cross-over genes 1: ";
    printGene(newIndividuals[0]);
    std::cout << std::endl << "Returning cross-over genes 2: ";
    printGene(newIndividuals[1]);
    std::cout << std::endl;*/
    return newIndividuals;
}

void Genetic::mutate(vector<Package* >* choosen) {

    std::uniform_real_distribution<double> rankUniform(0, rankTotal);
    std::uniform_real_distribution<double> mutationUniform(0, 1);


    float randomP;
    unsigned int randomSwap1, randomSwap2;
    bool different = false;

    for (unsigned int i = 0; i < choosen->size(); ++i) {
        randomP = mutationUniform(rngGenetic);

        if (randomP < mutation.deleteOld) {

            mutateDelete(choosen, i);
            return;
        }

        randomP -= mutation.deleteOld;

        if (randomP < mutation.insertNew) {

            mutateInsert(choosen, i);
            return;
        }

        randomP -= mutation.insertNew;

        if (randomP < mutation.inversion) {

            mutateInversion(choosen, i);
            return;
        }

        randomP -= mutation.inversion;

        if (randomP < mutation.swapOut) {
            mutateSwapNew(choosen, i);
            return;
        }

        randomP -= mutation.swapOut;

        // Swap genes within each parent.
        if (randomP < mutation.swapWithin) {
            mutateSwapWithin(choosen, i);
        }

        // Escaped with no mutation.

    }
    return;

}

void Genetic::mutateInsert(vector<Package *>* gene, unsigned int location) {
    // Size of gene
    unsigned int geneSize = gene->size();

    // Do we have any packages to insert?
    if (geneSize < numOfPackages) {

        std::uniform_int_distribution<int> geneUniform(0, geneSize - 1);
        std::uniform_int_distribution<int> packageUniform(0, numOfPackages - 1);

        unsigned int randomNewGene;

        // Is random package already present in gene
        bool present = true;

        // Limit to number of tries.
        unsigned int tries = 0;

        // Attempt to get non-duplicate package
        while(present == true && tries < 200) {
            // Assume not present in gene
            present = false;

            // Random package
            randomNewGene = packageUniform(rngGenetic); //rand() % numOfPackages;

            // Check gene for duplicates
            for(unsigned int swapIn = 0; swapIn < geneSize; ++swapIn) {
                if (packages[randomNewGene] == (*gene)[swapIn]) {
                    // Duplicate
                    present = true;
                    break;
                }
            }

            tries++;
        }

        if (tries < 200) {
            // Jam it in
            gene->insert(gene->begin() + location, packages[randomNewGene]);
        }

    }
    return;


}

void Genetic::mutateInversion(vector<Package* >* gene, unsigned int location) {
    // Size of gene
    unsigned int geneSize = gene->size();

    // Swap if we have genes to swap
    if (geneSize >= 2) {

        std::uniform_int_distribution<int> geneUniform(0, geneSize - 1);

        int randomSwap = location;

        // Find differing indices
        while(randomSwap == location) {
            randomSwap = geneUniform(rngGenetic);
        }

        int inversionSize = randomSwap - static_cast<int>(location);

        if (inversionSize < 0) {
            inversionSize *= -1;

            for (unsigned int i = 0; i < inversionSize / 2; ++i) {
                // Swap genes
                std::swap(gene->at(randomSwap + i), gene->at(location - i));
            }
        } else {
             for (unsigned int i = 0; i < inversionSize / 2; ++i) {
                // Swap genes
                std::swap(gene->at(location + i), gene->at(randomSwap - i));
            }
        }
    }
    return;
}


void Genetic::mutateDelete(vector<Package* >* gene, unsigned int location) {
    // Size of gene
    unsigned int geneSize = gene->size();

    // Delete a gene.
    // Only delete if there is a spare gene to delete.
    if (geneSize >= 2) {

        std::uniform_int_distribution<int> geneUniform(0, geneSize - 1);

        // Remove gene.
        gene->erase(gene->begin() + location);
    }

    return;
}


void Genetic::mutateSwapWithin(vector<Package *>* gene, unsigned int location) {
    // Size of gene
    unsigned int geneSize = gene->size();

    // Swap if we have genes to swap
    if (geneSize >= 2) {

        std::uniform_int_distribution<int> geneUniform(0, geneSize - 1);

        unsigned int randomSwap1 = 0;
        unsigned int randomSwap2 = 0;

        int randomSwap = location;

        // Find differing indices
        while(randomSwap == location) {
            randomSwap = geneUniform(rngGenetic);
        }

        // Swap genes
        std::swap(gene->at(randomSwap), gene->at(location));

    }
    // Return gene
    return;
}


void Genetic::mutateSwapNew(vector<Package* >* gene, unsigned int location) {
    unsigned int geneSize = gene->size();

    if (gene->size() < numOfPackages) {

        std::uniform_int_distribution<int> geneUniform(0, geneSize - 1);
        std::uniform_int_distribution<int> packageUniform(0, numOfPackages - 1);

        bool present = true;
        unsigned int tries = 0;
        unsigned int randomNewGene;

        while(present == true && tries < 200) {
            present = false;

            randomNewGene = packageUniform(rngGenetic);

            for(unsigned int swapIn = 0; swapIn < geneSize; ++swapIn) {
                if (packages[randomNewGene] == gene->at(swapIn)) {
                    present = true;
                    break;
                }
            }
            ++tries;
        }

        if (tries < 200) {
            gene->at(location) = packages[randomNewGene];
        }
    }

    return;

}

void Genetic::mergeLists(unsigned long i, unsigned long m, unsigned long j) {
    typedef pair<vector<Package* >, geneInfo> geneFit;
    vector< geneFit > geneBank;
    geneBank.resize(genes.size());

    unsigned long p = i;
    unsigned long q = m + 1;
    unsigned r = i;


    while(p <= m && q <= j) {


        if (genes[p].second.fitnessValue <= genes[q].second.fitnessValue) {
            geneBank[r] = genes[p];
            p++;
        } else {
            geneBank[r] = genes[q];
            q++;
        }
        r++;
    }

    while(p <= m) {
        geneBank[r] = genes[p];
        p++;
        r++;
    }

    while (q <= j) {
        geneBank[r] = genes[q];
        q++;
        r++;
    }

    for (r = i; r <= j; r++) {
        genes[r] = geneBank[r];
    }

    return;
}

void Genetic::mergeSort(unsigned long i, unsigned long j) {
    if (i == j) {
        return;
    }


    unsigned long m = (i + j) / 2;

    mergeSort(i, m);

    mergeSort(m + 1, j);

    mergeLists(i, m, j);

    return;
}
