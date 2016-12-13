#include "Genetic.h"

// Seed nice random number generator.
std::mt19937 rngGenetic(std::random_device{}());

Genetic::Genetic(std::vector<Package*> packs,
                 std::vector<vector<unsigned int> > matrix,
                 unsigned int weight,
                 unsigned int packLimit,
                 unsigned int population,
                 float stops, float drive, float shiftTime, unsigned long gens, mutation_struct mut)
                 : packages(packs), adMatrix(matrix), weightLimit(weight), packageLimit(packLimit),
                 stopTime(stops), driveTime(drive), timeLimit(shiftTime), generations(gens), mutation(mut)
{
    // Used to grab current package priority, to define totalPriority.
    Priority currentPriority;

    // Population number.
    popNum = population;

    // Debug output.
    std::cout << "Population: " << popNum << " Generations: " << gens << std::endl;

    // Total for roulette selection of mutation.
    mutation.total = mutation.deleteOld + mutation.insertNew + mutation.inversion + mutation.swapOut + mutation.swapWithin;

    // Create the best fitness info vector for later indexing.
    for (unsigned int i = 0; i < 5; ++i) {
        bestFitInfo.push_back(0);
    }

    // How many packages are there in the pool.
    numOfPackages = packages.size();

    // How many good members will be carried over into the next population
    elitist = mutation.elite * popNum;

    // Iterate over packages and grab total possible priority points.
    for (vector<Package* >::iterator iter = packages.begin(); iter != packages.end(); ++iter) {
        currentPriority = (*iter)->getPriority();

        // Grab priority weight and add to total.
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
    // Seed is initialized to genome size.
    size_t seed = gene->size();
    // Create hash
    for (auto iter = gene->begin(); iter != gene->end(); ++iter) {
        seed ^= (*iter)->getID() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    // Return hash
    return seed;
}

void Genetic::initPopulation() {
    // Package distribution
    std::uniform_int_distribution<int> packageUniform(0, numOfPackages - 1);

    // Individual we're creating randomly
    std::vector<Package* > individual;

    // Resize population to avoid push_backs
    genes.resize(popNum);

    // Gene info for current genome
    geneInfo currentGeneInfo;

    // Totals to keep track of for route, don't overload the truck.
    float totalWeight = 0;
    unsigned int totalPackages = 0;

    // Currently selected package
    Package* currentPackage;

    // Random package index
    unsigned int randomPackage;

    // Is package already present flag
    bool present = false;

    // Create population of individuals.
    for (unsigned int i = 0; i < popNum; i++) {

        // Clear individual
        individual.clear();

        // Don't overload the truck. Reset.
        totalWeight = 0;
        totalPackages = 0;

        // Loop while we still have packages to place, and we're under the limits.
        while (totalPackages < numOfPackages && totalPackages < packageLimit && totalWeight < weightLimit && (totalPackages * stopTime) < timeLimit / 2) {

            // Random index to packages
            randomPackage = packageUniform(rngGenetic);

            // Get the package pointer
            currentPackage = packages[randomPackage];

            // Assume not present in current individual
            present = false;

            // Loop over the individual looking for duplicate package
            for (std::vector<Package*>::iterator iter = individual.begin(); iter != individual.end(); ++iter) {
                // Check if duplicate
                if (*iter == currentPackage) {
                    // Duplicate!
                    present = true;
                    break;
                }
            }

            // If not a duplicate
            if (present == false) {

                // Add package to individual
                individual.push_back(currentPackage);
                totalPackages++;
                totalWeight += currentPackage->getWeight();
            }
        }

        // Optimize individual.
        // Incredibly slow!
        //individual = twoOpt(&individual);

        // Get gene info
        currentGeneInfo.fitnessValue = fitness(&individual);
        currentGeneInfo.hashValue = hash(&individual);
        currentGeneInfo.sizeValue = individual.size();

        // Add to gene pool
        genes[i] = std::make_pair(individual, currentGeneInfo);

    }

    // Create ranking array for selection
    initRanking(2);

    // Initially optimize random population.
    twoOptPopulation();
}

void Genetic::initRanking(float exponent) {
    // Resize the ranking vector used in selection to population.
    ranking.resize(genes.size());
    // Save size for later.
    rankingSize = ranking.size();

    // Load exponential ranking array. Always the same number of indices to individuals, so same ranking
    for (unsigned int i = 0; i < rankingSize; ++i) {
        // Exponential ranking
        ranking[i] = pow(i+1, exponent);
        // Add rank to total
        rankTotal += ranking[i];
    }
}

vector<double> Genetic::fitness(vector<Package* >* individual) {
    // Initialize fitness measurements
    // Total distance
    unsigned int distance = 0;
    // Total shift time
    unsigned int shiftTime = 0;
    // Total priorities in route.
    unsigned int priorities = 0;

    // Holder of current package priority
    Priority currentPriority;

    // Total weight of route
    double weight = 0;

    // Indices into adjacency matrix. Previous and current package.
    // Previous is set to warehouse which is always client 0.
    unsigned int previousIndex = 0, currentIndex = 0;

    // Current individual's fitness
    double indFit = 0;

    // Iterate over genome packages and gather fitness parameters.
    for (std::vector<Package*>::iterator iter = (*individual).begin(); iter != (*individual).end(); ++iter) {
        // Current package receiver ID for matrix.
        currentIndex = (*iter)->getReceiver()->getID();

        // Lookup distance between this and last package.
        distance += adMatrix[previousIndex][currentIndex];

        // Update previous to current for next round.
        previousIndex = currentIndex;

        // Get package priority
        currentPriority = (*iter)->getPriority();

        // Assign weight of current priority and add to total genome priority.
        if (currentPriority == Priority::OVERNIGHT) {
            priorities += OVERNIGHT_WEIGHT;
        } else if (currentPriority == Priority::TWO_DAY) {
            priorities += TWODAY_WEIGHT;
        } else {
            priorities += REGULAR_WEIGHT;
        }

        // Add current package weight to total genome weight.
        weight += (*iter)->getWeight();

        // Add package delivery time to total shift time.
        shiftTime += stopTime;
    }

    // Add the distance back to the warehouse after the last package.
    distance += adMatrix[previousIndex][0];

    // Figure out the drive time and add to shift time.
    shiftTime += distance * driveTime;

    // What generation are we on as a ratio to total generation?
    double generationRatio;

    // Only devalue fitness parameters for the first half of total generations.
    if (currentGeneration <  (generations + 1) / 2) {
        // Magic. Slightly devalue penalty fitness functions to allow bad individuals to explore problem space.
        generationRatio = static_cast<double>(currentGeneration + (generations + 1.0)/2.0)/static_cast<double>(generations);
    } else {
        // Later generations should stop exploring poorly fit individuals and optimize fit individuals.
        generationRatio = 1.0;
    }

    // Magic fitness for good individuals. Reward higher priorities and lesser shift times.
    indFit = pow(2.0, static_cast<double>(priorities) / static_cast<double>(totalPriority)) * pow(1.05, (static_cast<double>(individual->size() * stopTime) / static_cast<double>(shiftTime)));

    // Penalize over shift allowance
    if (shiftTime > timeLimit) {
        indFit -= (pow(2.0, 1.5 + (static_cast<double>(shiftTime) - static_cast<double>(timeLimit)) / static_cast<double>(timeLimit)) * generationRatio);
    }

    // Penalize over weight limit.
    if (weight > weightLimit) {
        indFit -= pow(2.0, 1.5 + (static_cast<double>(weight) - static_cast<double>(weightLimit)) / static_cast<double>(weightLimit)) * generationRatio;
    }

    // Penalize over package limit.
    if (individual->size() > packageLimit) {
        indFit -= pow(2.0, 1.5 + (static_cast<double>(individual->size()) - static_cast<double>(packageLimit)) / static_cast<double>(packageLimit)) * generationRatio;
    }

    // "Expand" very closely fit individuals.
    indFit = pow(indFit, 3.0);

    // Create fitness vector for return.
    vector<double> fit {indFit, static_cast<double>(priorities), static_cast<double>(distance), static_cast<double>(shiftTime), static_cast<double>(weight)};

    // Return fitness
    return fit;
}

void Genetic::loadPopulation(vector< pair<vector<Package* >, geneInfo> > newPopulation) {
    // Save incoming population to internal population.
    genes = newPopulation;
    // Update the size.
    popNum = newPopulation.size();

    // Initialize current fitness.
    vector<float> currentFitness (5, 0);

    // Initialize ranking for selection.
    initRanking(1.5);

    // Recompute the fitness values
    currentGeneration = 1;

    // Update fitness values due to resetting generation ratio in fitness function.
    for (auto iter = genes.begin(); iter != genes.end(); ++iter) {
        //(*iter).first = twoOpt(&(*iter).first);
        (*iter).second.fitnessValue = fitness(&(*iter).first);
    }

    // Sort population based on fitness value with lambda function.
    std::sort(genes.begin(), genes.end(), [] (pair<vector<Package* >, Genetic::geneInfo> const& left, pair<vector<Package* >, Genetic::geneInfo> const& right) {return left.second.fitnessValue < right.second.fitnessValue;});

    return;

}

vector< pair<vector<Package* >, Genetic::geneInfo> > Genetic::evolve_threads() {
    // Call evolve discarding returned fit individual.
    evolve();

    // Optimize population.
    twoOptPopulation();

    // Return ENTIRE population.
    return genes;

}

void Genetic::twoOptPopulation() {
    // Call out for debug
    std::cout << "*** Optimizing population Generation " << currentGeneration << " ***" << std::endl;

    // Iterate over population and invert optimize the genomes.
    for (auto iter = genes.begin(); iter != genes.end(); ++iter) {
        (*iter).first = twoOpt(&(*iter).first);
        (*iter).second.fitnessValue = fitness(&(*iter).first);
    }
}

void Genetic::printGene(vector<Package* >* gene) const {
       for (auto iter = gene->begin(); iter != gene->end(); ++iter) {
            std::cout << (*iter)->getID() << " ";
       }
}

vector<Package* > Genetic::evolve() {
    // Initialize and reserve fitness
    vector<double> currentFitness (5, 0);
    // Initialize and reserve best found individual info
    vector<double> currentBest (5, 0);

    // Temp gene
    vector<Package* > tempGene;
    // Temp gene info
    vector<double> tempInfo;

    // Current genome info
    geneInfo currentInfo;

    // Sadly, the use of NaN and -Inf are not guaranteed, so we'll use a dumb flag.
    // Have we loaded a best fitness individual yet?
    bool bestFoundYet = false;

    // Iterator for the population
    std::vector<std::pair<std::vector<Package* >, geneInfo> >::iterator row;

    // Evolve the population repeatedly.
    for (unsigned long i = 0; i < generations; ++i) {

        // What generation are we on?
        currentGeneration = i + 1;
        // Current best is reset each generation.
        currentBest[0] = 0;

        // Optimize the population occasionally, but not the last generation, we do that on the way out.
        if (currentGeneration % (generations / 4) == 0 && currentGeneration != generations) {
            twoOptPopulation();
        }

        // Loop through population
        for (row = genes.begin(); row != genes.end(); ++row) {

            // Check if this is our best gene. Update fitness if so.
            if (row->second.hashValue == bestGeneInfo.hashValue) {

                row->second.fitnessValue = fitness(&(row->first));
                bestGeneInfo = row->second;

            }

            // SLOW!
            //row->first = twoOpt(&(row->first));
            //row->second.fitnessValue = fitness(&(row->first));
            //row->second.hashValue = hash(&(row->first));

            // Is this the first individual?
            if (row->second.fitnessValue[0] > currentBest[0] || currentBest[0] == 0) {
                currentBest = row->second.fitnessValue;
            }

            // Found a better individual, or is this the first individual?
            if (bestFoundYet == false || row->second.fitnessValue[0] > bestGeneInfo.fitnessValue[0]) {
                bestFit = (*row).first;
                bestFitInfo = row->second.fitnessValue;
                bestGeneInfo = row->second;
                bestFoundYet = true;
            }
        }

        // Output information occasionally
        if (i % 500 == 0) {
            std::cout << "Generation " << i << " Best F: " << bestFitInfo[0] << " P: " << bestFitInfo[1] << "/" << totalPriority << " D: " << bestFitInfo[2] << " T: " << bestFitInfo[3] << "/" << timeLimit << " W: " << bestFitInfo[4] << "/" << weightLimit << " L: " << bestFit.size() << "/" << numOfPackages << " CO: " << avgIndividual << std::endl;//" F: " << genes[popNum - elitist - 2].second << " P: " << currentBest[1] << " D: " << currentBest[2] << " T: " << currentBest[3] << "/" << timeLimit << " W: " << currentBest[4] << "/" << weightLimit << std::endl;

            //tempGene = twoOpt(&bestFit);
            //tempInfo = fitness(&tempGene);
            //std::cout << "         2-opt Best F: " << tempInfo[0] << " P: " << tempInfo[1] << "/" << totalPriority << " D: " << tempInfo[2] << " T: " << tempInfo[3] << "/" << timeLimit << " W: " << tempInfo[4] << "/" << weightLimit << " L: " << tempInfo.size() << std::endl;
        }

        // Need to get the next population some how?!?!
        mate();
    }

    // Output stats for best evolved individuals.
    std::cout << "Best F: " << bestFitInfo[0] << " P: " << bestFitInfo[1] << "/" << totalPriority << " D: " << bestFitInfo[2] << " T: " << bestFitInfo[3] << "/" << timeLimit << " W: " << bestFitInfo[4] << "/" << weightLimit << " L: " << bestFit.size() << std::endl;

    // Grab the best fit individual and optimize them.
    bestFit = twoOpt(&bestFit);
    // Update fitness.
    bestFitInfo = fitness(&bestFit);

    // New best info
    std::cout << "2-opt Best F: " << bestFitInfo[0] << " P: " << bestFitInfo[1] << "/" << totalPriority << " D: " << bestFitInfo[2] << " T: " << bestFitInfo[3] << "/" << timeLimit << " W: " << bestFitInfo[4] << "/" << weightLimit << " L: " << bestFit.size() << std::endl;

    // Return the bestest.
    return getBest();
}

pair<int, int> Genetic::chooseParents() {
    // New population individuals
    vector<vector<Package* > > newIndividuals;
    // Resize appropriately to save push backs.
    newIndividuals.resize(2);

    // Uniform generator
    std::uniform_real_distribution<double> rankUniform(0, rankTotal);

    // Selections for random individuals
    int randomIndividual1 = 0;
    int randomIndividual2 = 0;

    // Ranks
    double rank1 = 0;
    double rank2 = 0;

    // Different individuals?
    bool different = false;

    // Loop until we get different individuals
    while (different == false) {
        // Reset chosen parents.
        randomIndividual1 = 0;
        randomIndividual2 = 0;

        // Grab random ranks for parent selection.
        rank1 = rankUniform(rngGenetic);
        rank2 = rankUniform(rngGenetic);

        // Loop through looking for selected individuals
        for (unsigned int i = 0; i < rankingSize; ++i) {
            // Did we find who we wanted?
            if (rank1 < ranking[i]) {
                // Choose this individual
                randomIndividual1 = i;
                // Update average selection for debugging.
                avgIndividual = avgIndividual * 100 + static_cast<float>(i);
                // Done searching.
                break;
            } else {

                // This was not the droid we were looking for.
                rank1 -= ranking[i];
            }

        }

        // See above...
        for (unsigned int i = 0; i < rankingSize; ++i) {
            if (rank2 < ranking[i]) {
                randomIndividual2 = i;
                avgIndividual = (avgIndividual + static_cast<float>(i)) / 102;

                // Done searching.
                break;
            } else {
                rank2 -= ranking[i];
            }

        }

        // Reset if we have different individuals.
        different = false;

        // Check hashes if we have different parents.
        if (genes[randomIndividual1].second.hashValue != genes[randomIndividual2].second.hashValue) {
            // These are different individuals.
            different = true;

        }

    }

    // Return pair of indices into genome vector.
    return make_pair(randomIndividual1, randomIndividual2);
}

vector<Package* > Genetic::twoOpt(vector<Package* >* gene) {
    // Size of gene
    unsigned int geneSize = gene->size();
    // Copies of gene for working on.
    vector<Package* > previousBestGene = *gene, workingGene = *gene;

    // Swap if we have genes to swap
    if (geneSize >= 2) {

        // Get fitness of working gene.
        vector<double> currentFitnessGene, previousFitness = fitness(&workingGene);

        // Did we improve the gene. Assume yes.
        bool improvement = true;

        // How big of an inversion are we doing.
        int inversionSize;

        // While we are improving, keep going!
        while (improvement == true) {

            // Assume we will have no improvement
            improvement = false;

            // Loop through gene looking for improvement. Front index of inversion
            for (unsigned int frontIndex = 0; frontIndex < geneSize - 2; ++frontIndex) {

                // Inner loop for end-point/back of inversion
                for (unsigned int backIndex = frontIndex + 1; backIndex < geneSize - 1; ++backIndex) {

                    // Grab inversion size for iterating.
                    inversionSize = backIndex - frontIndex;

                    // Perform the inversion
                    for (int index = 0; index < inversionSize / 2; ++index) {
                        // Swap genes
                        std::swap(workingGene.at(frontIndex + index), workingGene.at(backIndex - index));
                    }

                    // Check for improvement
                    currentFitnessGene = fitness(&workingGene);

                    // Check if distance improved
                    if (currentFitnessGene[2] < previousFitness[2]) {
                        //std::cout << "New distance: " << currentFitnessGene[2] << std::endl;

                        // Had an improvement, flag it
                        improvement = true;

                        // Save best score.
                        previousFitness = currentFitnessGene;

                        // Save best gene.
                        previousBestGene = workingGene;

                        // Break out of inside loop.
                        break;
                    } else {
                        // No improvement, restore previous best gene.
                        workingGene = previousBestGene;
                    }
                }

                // Check if we're improving.
                if (improvement == true) {
                    // Break out of outer loop.
                    break;
                }
            }
        }
    }

    // We are done improving, so return the improved gene. We send the previous gene since
    // we just messed with the current gene but it did not improve things.
    return previousBestGene;
}


void Genetic::mate() {

    // New population after mating and mutation.
    vector< pair<vector<Package* >, geneInfo> > newPopulation;
    // New individuals to load into the new population.
    vector< vector<Package* > > newIndividuals;
    // Parents to mate.
    pair<int, int> choosenParents;
    // Gene to mutate.
    vector<Package* >* choosenGene;
    // Current gene info for parents/children
    geneInfo currentGeneInfo1, currentGeneInfo2;

    // Resize to correct size to increase speed.
    newPopulation.resize(popNum);

    // Sort "in-place" based on fitness value. Least fit routes first in the vector, most fit last.
    std::sort(genes.begin(), genes.end(), [] (pair<vector<Package* >, Genetic::geneInfo> const& left, pair<vector<Package* >, Genetic::geneInfo> const& right) {return left.second.fitnessValue < right.second.fitnessValue;});

    // Assume not different for while loop.
    bool different = false;

    // Need to choose two parents for crossover, so half the crossover population.
    for (int i = 0; i < static_cast<int>(mutation.crossOver * popNum) / 2; ++i) {
        // Reset different
        different = false;

        // Choose two different parents
        while (different == false) {
            // Seed parents to be same
            choosenParents.first = 0;
            choosenParents.second = 0;

            // Loop until different parents are returned.
            while (choosenParents.first == choosenParents.second) {
                choosenParents = chooseParents();
            }

            // Create baby trucks based on parents.
            newIndividuals = crossOver(&genes.at(choosenParents.first).first, &genes.at(choosenParents.second).first);

            // Get size info
            currentGeneInfo1.sizeValue = newIndividuals[0].size();
            currentGeneInfo2.sizeValue = newIndividuals[1].size();

            // Incredibly slow!
            //newIndividuals[0] = twoOpt(&newIndividuals[0]);
            //newIndividuals[1] = twoOpt(&newIndividuals[1]);

            // Get hash info
            currentGeneInfo1.hashValue = hash(&newIndividuals[0]);
            currentGeneInfo2.hashValue = hash(&newIndividuals[1]);

            // Make sure baby routes really are unique snowflakes.
            if (currentGeneInfo1.hashValue != currentGeneInfo2.hashValue) {
                // They were unique compared to each other.
                different = true;
                // Get fitness now instead of in a fitness iterator. Saves memory read.
                currentGeneInfo1.fitnessValue = fitness(&newIndividuals[0]);
                currentGeneInfo2.fitnessValue = fitness(&newIndividuals[1]);
            }

        }

        // Stuff new individuals into new population.
        newPopulation[2 * i] = make_pair(newIndividuals[0], currentGeneInfo1);
        newPopulation[2 * i + 1] = make_pair(newIndividuals[1], currentGeneInfo2);
    }

    // Copy over those fitter individual to fill up until we hit the reserved elite cap. We will mutate these better individuals.
    for (int i = static_cast<int>(mutation.crossOver * popNum) / 2; i < static_cast<int>(popNum - elitist); ++i) {
        newPopulation[i] = genes[i];
    }

    // Create a new population not including saved elite individuals. Probably should allow mutation, but mutation replaces individuals.
    // Crikey!
    for (int i = 0; i < static_cast<int>(popNum - elitist); ++i) {

        // Pick our current gene
        choosenGene = &newPopulation[i].first;

        // Reset different
        different = false;

        // Loop until mutation is different
        while (different == false) {
            // Mutate individual
            mutate(choosenGene);

            // Calculate hash of mutation.
            currentGeneInfo1.hashValue = hash(choosenGene);

            // Seed different is true.
            different = true;

            // Iterate over new population, looking for duplicate individuals.
            for (auto iter = newPopulation.begin(); iter != newPopulation.end(); ++iter) {
                // Check uniqueness.
                if (currentGeneInfo1.hashValue == (*iter).second.hashValue) {
                    // Duplicate
                    different = false;
                    break;
                }
            }

            // If different than new population.
            if (different == true) {
                // Is this an individual that we just saw? Diversity is good they say!
                for (auto iter = genes.begin(); iter != genes.end(); ++iter) {
                    if (currentGeneInfo1.hashValue == (*iter).second.hashValue) {
                        // Nope, not diverse enough.
                        different = false;
                        break;
                    }
                }
            }
        }

        // We've created unique snowflakes, save them. Get info.
        currentGeneInfo1.sizeValue = choosenGene->size();
        currentGeneInfo1.fitnessValue = fitness(choosenGene);

        // Save to population.
        newPopulation[i] = make_pair(*choosenGene, currentGeneInfo1);
    }

    // Save the elite few, the fittest. Save the Queen!
    for (unsigned int i = (popNum - elitist - 1); i < popNum; ++i) {
        newPopulation[i] = genes[i];
    }

    // Assign the new population to replace the old one.
    genes = newPopulation;
}

vector<vector<Package*> > Genetic::crossOver(vector<Package* >* gene1, vector<Package* >* gene2) {

    // Do we have genes of size 1?
    if (gene1->size() <= 1 || gene2->size() <= 1) {
        return vector<vector<Package* > > {*gene1, *gene2};
    }

    // Size of genes.
    unsigned int geneLength1 = gene1->size();
    unsigned int geneLength2 = gene2->size();

    // Maps to hold what genes are within each vector for excluding duplicate genes.
    std::unordered_map<Package*,bool> gene1Map;
    std::unordered_map<Package*,bool> gene2Map;


    std::unordered_map<Package*,bool>::const_iterator gene1MapIter;
    std::unordered_map<Package*,bool>::const_iterator gene2MapIter;

    std::vector<Package* >::iterator gene1End = gene1->end();
    std::vector<Package* >::iterator gene2End = gene2->end();

    // Lengths of genes
    unsigned int smallestLength, longestLength;

    // Random crossover point.
    unsigned int randomPoint;

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

    // Size the hash table appropriately for the max number of elements.
    gene1Map.reserve(longestLength);
    gene2Map.reserve(longestLength);

    std::uniform_int_distribution<int> pointUniform(1, smallestLength - 1);

    // Random point to apply crossover. Don't want to cross over at 0
    randomPoint = pointUniform(rngGenetic); //(rand() % (smallestLength - 1)) + 1;

    // Resize to eliminate push-backs during initial crossover
    packageGene1.resize(randomPoint + 1);
    packageGene2.resize(randomPoint + 1);

    // Genes crosses over from 0 to randomPoint in genes
    for (unsigned int i = 0; i <= randomPoint; ++i) {
        packageGene1[i] = (*gene2)[i];
        gene1Map.insert({packageGene1[i], true});
        packageGene2[i] = (*gene1)[i];
        gene2Map.insert({packageGene2[i], true});
    }


    // Fill map
    // Fill individual[0] genes based on the size of gene1 in the order they appear
    for (vector<Package* >::iterator gene1_iter = gene1->begin(); gene1_iter != gene1End; ++gene1_iter) {

        // Check for duplicate genes.
        gene1MapIter = gene1Map.find(*gene1_iter);

        // If not duplicate iterator will be at the end.
        if (gene1MapIter == gene1Map.end()) {
            // We're golden, so insert gene and add to map.
            packageGene1.push_back(*gene1_iter);
            gene1Map.insert({*gene1_iter, true});
        }
    }

    // Fill individual[0] genes based on the size of gene1 in the order they appear
    for (vector<Package* >::iterator gene2_iter = gene2->begin(); gene2_iter != gene2End; ++gene2_iter) {

        // Check for duplicate genes.
        gene2MapIter = gene2Map.find(*gene2_iter);

        // If not duplicate iterator will be at the end.
        if (gene2MapIter == gene2Map.end()) {
            // We're golden, so insert gene and add to map.
            packageGene2.push_back(*gene2_iter);
            gene2Map.insert({*gene2_iter, true});
        }

    }

    // Assign to return vector
    newIndividuals[0] = packageGene1;
    newIndividuals[1] = packageGene2;

    // Return new children.
    return newIndividuals;
}

void Genetic::mutate(vector<Package* >* choosen) {

    // Mutation rank generator
    std::uniform_real_distribution<double> rankUniform(0, rankTotal);
    // Uniform mutation generator
    std::uniform_real_distribution<double> mutationUniform(0, 1);

    // Random number used to select mutation
    float randomP;

    // Loop through each gene in genome. Each gene has a chance to be chosen for a mutation.
    for (unsigned int i = 0; i < choosen->size(); ++i) {
        // Pick a potential random mutation
        randomP = mutationUniform(rngGenetic);

        // Start rank selecting mutation.
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

        unsigned int randomSwap = location;

        // Find differing indices
        while(randomSwap == location) {
            randomSwap = geneUniform(rngGenetic);
        }

        int inversionSize = randomSwap - static_cast<int>(location);

        if (inversionSize < 0) {
            inversionSize *= -1;

            for (int i = 0; i < inversionSize / 2; ++i) {
                // Swap genes
                std::swap(gene->at(randomSwap + i), gene->at(location - i));
            }
        } else {
             for (int i = 0; i < inversionSize / 2; ++i) {
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

        unsigned int randomSwap = location;

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

    // Make sure we have something to swap!
    if (geneSize < numOfPackages) {

        std::uniform_int_distribution<int> geneUniform(0, geneSize - 1);
        std::uniform_int_distribution<int> packageUniform(0, numOfPackages - 1);

        // Gene already present?
        bool present = true;
        // Number of tries to select new gene
        unsigned int tries = 0;
        // Random gene
        unsigned int randomNewGene;

        // Attempt to get a unique gene
        while(present == true && tries < 200) {
            // Reset uniqueness
            present = false;

            // Pick gene
            randomNewGene = packageUniform(rngGenetic);

            // Check for uniqueness in current genome
            for(unsigned int swapIn = 0; swapIn < geneSize; ++swapIn) {
                if (packages[randomNewGene] == gene->at(swapIn)) {
                    // Duplicate
                    present = true;
                    break;
                }
            }
            // Try again
            ++tries;
        }

        // Did we give up from not finding a unique gene?
        if (tries < 200) {
            // Must have found a unique gene. replace with new gene.
            gene->at(location) = packages[randomNewGene];
        }
    }

    return;

}

// Not used
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

// Not used.
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
