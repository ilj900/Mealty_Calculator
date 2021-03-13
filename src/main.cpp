#include "main.h"
#include "json.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <functional>
#include <chrono>

using json = nlohmann::json;

// Parameters
static float MaximumMealCalories = 2000.f;
static float MinimumMealCalories = 1900.f;
static float MinMealFactor = 3.5f;
static int MinMealPerDay = 4;
static int MaxMealsPerDay = 5;
static int Days = 5;
static std::vector<std::string> CategoriesToSkip = {"food", "almost_ready"};
static std::vector<std::string> DishesToSkip = {"Хлеб", "хлеб", "Булочка", "булочка", "Тартин", "тартин"};  // Add lowercase strings here, C++ has no built-in function to transform strings to lowercase

struct MenuItem
{
public:
    MenuItem(): Id(Counter++) {};
    unsigned int Id;
    std::string Name = "";
    std::string AdditionalName = "";
    std::string Category = "";
    float Weight = 0.f;
    float CaloriesPer100 = 0.f;
    float TotalCalories = 0.f;
    float Carbohydrates = 0.f;
    float Proteins = 0.f;
    float Fats = 0.f;
    float Price = 0.f;
    float Factor = 0.f;
    bool Available = false;

    static unsigned int Counter;

    bool operator==(const MenuItem& Other) {return Other.Id == Id;}
};

unsigned int MenuItem::Counter = 0u;

void RecursiveComposition(std::vector<std::vector<MenuItem>>& Solutions, std::vector<MenuItem>& Menu, size_t StartingIndex, std::vector<MenuItem> &DailyRation, float TotalCalories, float TotalPrice, bool DesertIncluded)
{
    for (std::size_t i = StartingIndex; i < Menu.size(); ++i)
    {
        MenuItem& Meal = Menu[i];
        DailyRation.push_back(Meal);
        TotalCalories += Meal.TotalCalories;
        TotalPrice += Meal.Price;
        bool IsDessert = Meal.Category == "drink" || Meal.Category == "bread" || Meal.Category == "snack";

        // If DailyRation has more calories than we need
        // Or if DailyRation has more dishes than needed
        // Or if it's second dessert
        if ((TotalCalories > MaximumMealCalories) || (DailyRation.size() >= MaxMealsPerDay) || (IsDessert && DesertIncluded))
        {
            DailyRation.pop_back();
            TotalCalories -= Meal.TotalCalories;
            TotalPrice -= Meal.Price;
            continue;
        }
        // If Meal has more calories that minimum then we need to check it
        if (TotalCalories > MinimumMealCalories)
        {
            // If meal's factor fits then we save it
            if (TotalCalories / TotalPrice > MinMealFactor && DailyRation.size() >= MinMealPerDay)
            {
                Solutions.push_back(DailyRation);
            }
            DailyRation.pop_back();
            TotalCalories -= Meal.TotalCalories;
            TotalPrice -= Meal.Price;
            continue;
        }

        RecursiveComposition(Solutions, Menu, i + 1, DailyRation, TotalCalories, TotalPrice, IsDessert);

        DailyRation.pop_back();
        TotalCalories -= Meal.TotalCalories;
        TotalPrice -= Meal.Price;
    }

    return;
}

std::vector<std::vector<MenuItem>> GenerateWeeklyRation(std::vector<std::vector<MenuItem>>& Solutions)
{
    std::vector<std::vector<MenuItem>> WeeklyRation;

    for (int i = 0; i < 5; ++i)
    {
        std::mt19937 Generator;
        Generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> Distribution(0,int(Solutions.size() - 1));
        auto RandomDailyMeal = std::bind(Distribution, Generator);

        int Index = RandomDailyMeal();
        auto DailyRation = Solutions[Index];
        WeeklyRation.push_back(DailyRation);

        for (auto Meal : DailyRation)
        {
            for(int i = 0; i < Solutions.size(); ++i)
            {
                for (auto Single : Solutions[i])
                {
                    if (Single == Meal)
                    {
                        if (Solutions.size() == 0)
                        {
                            // TODO
                            throw std::runtime_error("Out of solutions!");
                        }
                        Solutions[i] = Solutions.back();
                        Solutions.pop_back();
                        --i;
                        break;
                    }
                }
            }
        }
    }

    return WeeklyRation;
}

void from_json(const json& Json, MenuItem& Item)
{
    Json.at("name").get_to(Item.Name);
    Json.at("additional_name").get_to(Item.AdditionalName);
    Json.at("category").get_to(Item.Category);
    Json.at("weight").get_to(Item.Weight);
    Json.at("calories_per_100").get_to(Item.CaloriesPer100);
    Json.at("total_calories").get_to(Item.TotalCalories);
    Json.at("carbohydrates").get_to(Item.Carbohydrates);
    Json.at("proteins").get_to(Item.Proteins);
    Json.at("fats").get_to(Item.Fats);
    Json.at("price").get_to(Item.Price);
    Json.at("factor").get_to(Item.Factor);
    Json.at("available").get_to(Item.Available);
}

int main(int argc, char* argv[])
{
    system("chcp 65001");

    std::vector<MenuItem> Menu;

    // Load menu
    std::ifstream File("../Menu.json", std::ifstream::in);
    if (!File.is_open())
        return -1;
    json Json;
    File >> Json;
    auto MenuJson = Json["Menu"];
    for (auto MenuItemJson : MenuJson)
    {
        MenuItem Item = MenuItemJson.get<MenuItem>();
        Menu.push_back(Item);
    }

    // Filter menu
    for (int i = 0; i < Menu.size(); ++i)
    {
        // If MenuItem is out of stock then remove it
        if (!Menu[i].Available)
        {
            Menu[i] = Menu.back();
            Menu.pop_back();
            --i;
            continue;
        }

        // Exclude dishes from categories we don't want
        for (auto& CategoryName : CategoriesToSkip)
        {
            if (Menu[i].Category == CategoryName)
            {
                Menu[i] = Menu.back();
                Menu.pop_back();
                --i;
                continue;
            }
        }

        // Exclude dishes we don't want
        for (auto& DishName : DishesToSkip)
        {
            if (Menu[i].Name.find(DishName) != std::string::npos)
            {
                Menu[i] = Menu.back();
                Menu.pop_back();
                --i;
                continue;
            }
        }
    }

    // Sort so the drinks, bread and snacks are first
    int l = 0;
    int r = Menu.size() - 1;
    while(l < r)
    {
        while(Menu[l].Category == "drink" || Menu[l].Category == "bread" || Menu[l].Category == "snack")
            ++l;
        while(!(Menu[r].Category == "drink" || Menu[r].Category == "bread" || Menu[r].Category == "snack"))
            --r;
        auto T = Menu[l];
        Menu[l] = Menu[r];
        Menu[r] = T;
    }

    std::cout<< "Menu enlists " << Menu.size() << " positions." << std::endl;

    std::vector<std::vector<MenuItem>> Solutions;
    float TotalCalories = 0.f;
    float TotalPrice = 0.f;
    bool Drink = false;
    static std::vector<MenuItem> DailyRation;
    RecursiveComposition(Solutions, Menu, 0, DailyRation, TotalCalories, TotalPrice, Drink);

    std::cout << "Total of " << Solutions.size() << " daily rations found." << std::endl;
    for (int i = 0; i <= MaxMealsPerDay; ++i)
    {
        int j = 0;
        for (auto& Solution : Solutions)
        {
            if (Solution.size() == i)
            {
                ++j;
            }
        }
        if (j != 0) {
            std::cout << j << " Daily rations with " << i << " meals." << std::endl;
        }
    }

    auto WeeklyRation = GenerateWeeklyRation(Solutions);

    std::ofstream OutputFile;
    OutputFile.open("Ration.txt");
    if (!OutputFile.is_open())
    {
        return -1;
    }
    for (int i = 0; i < WeeklyRation.size(); ++i)
    {
        float DailyCalories = 0.f;
        float DailyPrice = 0.f;
        OutputFile << "Day " << i+1 << std::endl;
        for (int j = 0; j < WeeklyRation[i].size(); ++j)
        {
            OutputFile << j+1 << ": " << WeeklyRation[i][j].Name << " " << WeeklyRation[i][j].AdditionalName << std::endl;
            DailyCalories += WeeklyRation[i][j].TotalCalories;
            DailyPrice += WeeklyRation[i][j].Price;
        }
        OutputFile << "Total daily calories:" << DailyCalories << std::endl;
        OutputFile << "Total price: " << DailyPrice << std::endl;
        OutputFile << "Factor: " << DailyCalories / DailyPrice << std::endl << std::endl;
    }
    OutputFile.close();

    return 0;
}