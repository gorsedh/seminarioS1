/*This file contains all the functions needed for the execution of main.cpp*/

#include "functions.hpp"

#include <cassert>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////
//Data collecting functions
////////////////////////////////////////////////////////////////////////////////////////////////

void dataPrint(Population const &pop)
{
    std::cout << "S = " << pop.susceptiblesCounter() << ", I = " << pop.infectsCounter() << ", R = " << pop.recoveredCounter() << "\n";
}

void dataCollecting(Population const &pop, std::vector<dailyReport> &finalReport)
{
    dailyReport day = dailyReport(pop.susceptiblesCounter(), pop.infectsCounter(), pop.recoveredCounter());
    finalReport.push_back(day);
}

//create and write the report.txt
void printDataToFile(std::vector<dailyReport> const &finalReport)
{
    int const population_size = (finalReport[1].daily_S_ + finalReport[1].daily_I_ + finalReport[1].daily_R_);
    std::ofstream report;
    report.open("report.txt");
    report << "//Population size is " << population_size << ", beta is " << beta << ", Gamma is " << gamma_ << "\n";
    for (size_t i = 0; i < finalReport.size(); i++)
    {
        report << i + 1 << std::setw(10) << finalReport[i].daily_S_ << std::setw(10) << finalReport[i].daily_I_ << std::setw(10) << finalReport[i].daily_R_ << std::setw(10) << "\n";
    }
    report.close();
}

////////////////////////////////////////////////////////////////////////////////////////////////
//Printing on terminal function
////////////////////////////////////////////////////////////////////////////////////////////////

void gridPrint(Population const &pop)
{ //sistemare gli output
    std::cout << "   ";
    for (int edge = 0; edge < pop.getSize(); ++edge)
    {
        // labels on vertical edge
        std::cout << std::right << std::setw(2) << edge << " ";
    }
    std::cout << '\n';

    for (int row = 0; row < pop.getSize(); ++row)
    {
        // labels on horizontal edge
        std::cout << std::right << std::setw(2) << row << "  ";

        for (int column = 0; column < pop.getSize(); ++column)
        {
            auto cell = pop.getBoard()[row * pop.getSize() + column];

            // cells on the edge o the board are counted as "susceptible".
            switch (cell)
            {
            case (Condition::S):
                std::cout << std::setw(1) << "O"
                          << "  ";
                break;

            case (Condition::I):
                std::cout << std::setw(1) << "\033[31mI\033[0m"
                          << "  ";
                break;

            case (Condition::R):
                std::cout << std::setw(1) << "\033[36mX\033[0m"
                          << "  ";
                break;

            case (Condition::E):
                std::cout << std::setw(1) << "\033[30mE\033[0m"
                          << "  ";
                break;
            default:
                std::cout << "no cell selected" << '\n';
            }
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

////////////////////////////////////////////////////////////////////////////////////////////////
//Adjacent infects
////////////////////////////////////////////////////////////////////////////////////////////////

//counts the number of infects adjacent to a cell (in the 8 adjacent cells)
int adjacentInfects(Population &pop, int row, int column)
{
    //assert(row != 0 && column != 0);
    auto cell = pop(row, column);
    int result = 0;
    //count only adjacents, not the central one
    for (int i = row - 1; i != row + 2; ++i)
    {
        for (int j = column - 1; j != column + 2; ++j)
        {
            if (pop(i, j) == Condition::I)
            {
                result++;
            }
        }
    }
    switch (cell)
    {
    case Condition::I:
        return (result - 1);
        break;
    default:
        return result;
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
//Spreading functions
////////////////////////////////////////////////////////////////////////////////////////////////

Population linearSpread(Population &previous)
{ //includere uno spread in cui i valori di gamma_ e beta variano a seconda del numero di malati?
    int size = previous.getSize();
    Population evolved(size);
    std::random_device seed;
    std::mt19937 gen(seed());
    std::uniform_real_distribution<> dis(0, 1);

    for (int row_loop = 1; row_loop < size - 1; ++row_loop)
    {
        for (int column_loop = 1; column_loop < size - 1; ++column_loop)
        {
            assert(row_loop != 0 && row_loop != size && column_loop != 0 && column_loop != size);

            auto cell = previous(row_loop, column_loop);
            switch (cell)
            {
            case Condition::S:
            {
                int i = 0;
                while (i != adjacentInfects(previous, row_loop, column_loop))
                {
                    ++i;
                }
                if (i == adjacentInfects(previous, row_loop, column_loop) &&
                    dis(gen) <= i * beta) //modello di spread lineare fra (0,0) e (1,beta)
                // dis(gen) <= booleanMarker(i) * ( (i-1)*(1-beta)/7 + beta) )  //linear spread fra (1,beta) e (8,1) con annullamento sullo zero
                //commentare a scelta una delle due linee precedenti/choose one of the two previous lines to commentate
                {
                    evolved(row_loop, column_loop) = (Condition::I);
                }
                break;
            }
            case Condition::I:
            {
                if (dis(gen) <= gamma_)
                {

                    evolved(row_loop, column_loop) = (Condition::R);
                }
                else
                {
                    evolved(row_loop, column_loop) = previous(row_loop, column_loop);
                }
                break;
            }
            case Condition::R:
            {
                evolved(row_loop, column_loop) = previous(row_loop, column_loop);
                break;
            }

            case Condition::E:
            {
                evolved(row_loop, column_loop) = previous(row_loop, column_loop);
                break;
            }
            default:
                break;
            }
        }
    }
    return evolved;
}

//these functions move cells
void cellMover(Population &pop)
{
    int iX, jX, iY, jY;
    int redSize = static_cast<int>(pop.getSize()) - 2; //avoid swapping cells on border
    int numOfSwaps = floor((redSize * redSize)/32);
    for (int i = 0; i < numOfSwaps; i++)
    {
        iX = (rand() + time(nullptr)) % (redSize) + 1;
        jX = (rand() + time(nullptr)) % (redSize) + 1;
        iY = (rand() + time(nullptr)) % (redSize) + 1;
        jY = (rand() + time(nullptr)) % (redSize) + 1;

        pop.swapTwoCells(iX, jX, iY, jY);
    }
}


void adjacentMover(Population &pop)
{
    int iX, jX, iY, jY;
    int redSize = static_cast<int>(pop.getSize()) - 4; //avoid swapping cells on border
    int numOfSwaps = floor((redSize * redSize) / 10);
    for (int i = 0; i < numOfSwaps; i++)
    {
        iX = (rand() + time(nullptr)) % (redSize) + 2;
        if (iX / redSize < 0.5)
        {
            jX = iX + 1;
        }
        else
        {
            jX = iX - 1;
        }
        iY = (rand() + time(nullptr)) % (redSize) + 2;
        if (jX / redSize < 0.5)
        {
            jY = iY - 1;
        }
        else
        {
            jY = iY + 1;
        }
        pop.swapTwoCells(iX, jX, iY, jY);
    }
}

//this function is used for the linear #2 spread model, self explanatory
int booleanMarker(int x)
{
    if (x == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
//Check function
////////////////////////////////////////////////////////////////////////////////////////////////

//this function resets the values of global variables at the end of every day
void checkParameters(ParametersCheck const &check)
{
    if (beta != check.betaCheck_)
    {
        beta = check.betaCheck_;
    }
    if (gamma_ != check.gammaCheck_)
    {
        gamma_ = check.gammaCheck_;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
//execution functions
////////////////////////////////////////////////////////////////////////////////////////////////

//these functions allow the execution of the program
std::vector<dailyReport> execute(ParametersCheck const &check, int const size)
{
    Population pop(size);
    std::vector<dailyReport> finalReport;
    emptyBoard(pop);
    initializeCornerInfect(pop); //add initialization settings
    cellEmptier(pop, check, size);
    bool cellMove = check.cellMove_;
    int dayspassed = 1;
    //far andare la funzione autonomamente
    while (pop.infectsCounter() != 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (cellMove == 1)
        {
            if (dayspassed < 50)
            {
                cellMover(pop);
            }
        } //comment this line to stop the moving of cells
        pop = linearSpread(pop);
        std::cout << "Day: " << dayspassed << "\n";
        gridPrint(pop);
        dataPrint(pop);
        dataCollecting(pop, finalReport);
        std::cout << "\n";
        dayspassed++;
        checkParameters(check);
    }
    return finalReport;
}

std::vector<dailyReport> noGraphicsExecute(ParametersCheck const &check, int const size)
{
    Population pop(size);
    std::vector<dailyReport> finalReport;
    emptyBoard(pop);
    initializeInfect(pop); //add initialization settings
    cellEmptier(pop, check, size);
    bool cellMove = check.cellMove_;
    int dayspassed = 1;
    //far andare la funzione autonomamente
    while (pop.infectsCounter() != 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (cellMove == 1)
        {
            cellMover(pop);
        } //comment this line to stop the moving of cells
        pop = linearSpread(pop);
        std::cout << "Day: " << dayspassed << "\n";
        dataPrint(pop);
        dataCollecting(pop, finalReport);
        std::cout << "\n";
        dayspassed++;
        checkParameters(check);
    }
    return finalReport;
}

std::vector<dailyReport> bigSimulationExecute(ParametersCheck const &check, int const size)
{
    Population pop(size);
    std::vector<dailyReport> finalReport;
    emptyBoard(pop);
    initializeInfect(pop);
    cellEmptier(pop, check, size);
    bool cellMove = check.cellMove_;
    while (pop.infectsCounter() != 0)
    {
        if (cellMove == 1)
        {
            cellMover(pop);
        } //comment this line to stop the moving of cells
        pop = linearSpread(pop);
        std::cout << "\n";
        dataCollecting(pop, finalReport);
        checkParameters(check);
    }
    return finalReport;
}

std::vector<dailyReport> bigSimulationExecuteMED(ParametersCheck const &check, int const size)
{
    Population pop(size);
    std::vector<dailyReport> finalReport;
    emptyBoard(pop);
    initializeInfect(pop);
    cellEmptier(pop, check, size);
    bool cellMove = check.cellMove_;
    while (pop.infectsCounter() != 0)
    {
        if (cellMove == 1)
        {
            adjacentMover(pop);
        } //comment this line to stop the moving of cells
        pop = linearSpread(pop);
        std::cout << "\n";
        dataCollecting(pop, finalReport);
        checkParameters(check);
    }
    return finalReport;
}
