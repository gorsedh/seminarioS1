#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <cmath>

struct dailyReport
{
    float daily_S_;
    float daily_I_;
    float daily_R_;

    dailyReport();
    dailyReport(float iS, float iI, float iR) : daily_S_(iS), daily_I_(iI), daily_R_(iR) {}
};

double population;
double beta;
double gamma_;
int daysPassed = 0;
std::vector<dailyReport> totalReport;

void inputData()
{
    std::cout << "Popolazione: ";
    std::cin >> population;
    std::cout << "\nBeta: ";
    std::cin >> beta;
    std::cout << "\nGamma: ";
    std::cin >> gamma_;
}

void printDataToFile(std::vector<dailyReport> const &finalReport)
{
    int const population_size = (finalReport[1].daily_S_ + finalReport[1].daily_I_ + finalReport[1].daily_R_);
    std::ofstream report;
    report.open("report.txt");
    report << "//Population size is " << population_size << ", beta is " << beta << ", Gamma is " << gamma_ << "\n";
    for (size_t i = 0; i < finalReport.size(); i++)
    {
        report << i + 1 << std::setw(10) << finalReport[i].daily_S_ << std::setw(10) << finalReport[i].daily_I_ << std::setw(10) << finalReport[i].daily_R_ << std::setw(10) /*<< 0 << std::setw(10)*/ << "\n";
    }
    report.close();
}

int main()
{
    inputData();
    dailyReport day0 = dailyReport(population - 2, 2, 0);

    totalReport.push_back(day0);

    while (totalReport[daysPassed].daily_I_ > 0.5)
    {
        double todayS, todayI, todayR;
        double incrementI, incrementR;

        incrementI = totalReport[daysPassed].daily_S_ * totalReport[daysPassed].daily_I_ * beta / population;
        incrementR = totalReport[daysPassed].daily_I_ * gamma_;

        todayS = totalReport[daysPassed].daily_S_ - incrementI;
        if(todayS < 0){
            todayS = 0;
        }
        todayI = totalReport[daysPassed].daily_I_ + incrementI - incrementR;
        if(todayI < 0){
            todayI = 0;
        }
        todayR = totalReport[daysPassed].daily_R_ + incrementR;
        if(todayR > population){
            todayR = population;
        }

        dailyReport today = dailyReport(todayS, todayI, todayR);
        totalReport.push_back(today);

        daysPassed++;
        std::cout << incrementI << " " << incrementR << '\n';
    }

    printDataToFile(totalReport);
}