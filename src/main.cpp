#include "main.h"
#include "CLI11/CLI.hpp"

#include <iostream>
#include <vector>

struct MenuItem
{
    std::string Name;
    std::string Category;
    float Weight;
    float CaloriesPer100;
    float TotalCalories;
    float Carbohydrates;
    float Proteins;
    float Fats;
    float Price;
    float Factor;
};

int main(int argc, char* argv[])
{
    CLI::App App{"Parse parameters"};

    std::vector<std::string> Names;
    App.add_option<std::vector<std::string>>("--Names", Names);

    std::vector<std::string> Categories;
    App.add_option<std::vector<std::string>>("--Categories", Categories);

    std::vector<float> Weights;
    App.add_option<std::vector<float>>("--Weights", Weights);

    std::vector<float> CaloriesPer100;
    App.add_option<std::vector<float>>("--CaloriesPer100", CaloriesPer100);

    std::vector<float> TotalCalories;
    App.add_option<std::vector<float>>("--TotalCalories", TotalCalories);

    std::vector<float> Carbohydrates;
    App.add_option<std::vector<float>>("--Carbohydrates", Carbohydrates);

    std::vector<float> Proteins;
    App.add_option<std::vector<float>>("--Proteins", Proteins);

    std::vector<float> Fats;
    App.add_option<std::vector<float>>("--Fats", Fats);

    std::vector<float> Prices;
    App.add_option<std::vector<float>>("--Prices", Prices);

    std::vector<float> Factors;
    App.add_option<std::vector<float>>("--Factors", Factors);

    CLI11_PARSE(App, argc, argv);

    if (!(Factors.size() == Prices.size() == Fats.size() == Proteins.size()
    == Carbohydrates.size() == TotalCalories.size() == CaloriesPer100.size() == Weights.size() == Categories.size() == Names.size()))
        return -1;

    std::vector<MenuItem> MenuItems(Names.size());
    for (int i = 0; i < MenuItems.size(); ++i)
    {
        MenuItems[i] = {Names[i], Categories[i], Weights[i], CaloriesPer100[i], TotalCalories[i], Carbohydrates[i], Proteins[i], Fats[i], Prices[i], Factors[i]};
    }

}