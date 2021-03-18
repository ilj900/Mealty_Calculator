#include "main.h"
#include "json.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <functional>
#include <chrono>
#include <map>

using json = nlohmann::json;

// Parameters
static float MaximumMealCalories = 2000.f;
static float MinimumMealCalories = 1900.f;
static float MinMealFactor = 3.5f;
static int MinMealPerDay = 4;
static int MaxMealsPerDay = 5;
static int Days = 5;
static std::vector<std::string> CategoriesToSkip = {"food", "almost_ready"};
static std::vector<std::string> DishesToSkip = {"Хлеб", "хлеб", "Булочка", "булочка", "Тартин", "тартин", "Оливье", "оливье", "Пирожки", "пирожки",
                                                "Торт", "торт", "Напиток", "напиток", "Сэндвич", "сэндвич", "Бургер", "бургер", "Капрезе", "капрезе",
                                                "Кус-кус", "кус-кус", "Булгур", "булгур", "Кекс", "кекс", "Фондан", "фондан"};  // Add lowercase strings here, C++ has no built-in function to transform russian strings to lowercase

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
    static bool CompareByWeight(const MenuItem& Var1, const MenuItem& Var2) {return Var1.Weight > Var2.Weight;}
    static bool CompareByCaloriesPer100(const MenuItem& Var1, const MenuItem& Var2) {return Var1.CaloriesPer100 > Var2.CaloriesPer100;}
    static bool CompareByTotalCalories(const MenuItem& Var1, const MenuItem& Var2) {return Var1.TotalCalories > Var2.TotalCalories;}
    static bool CompareByCarbohydrates(const MenuItem& Var1, const MenuItem& Var2) {return Var1.Carbohydrates > Var2.Carbohydrates;}
    static bool CompareByProteins(const MenuItem& Var1, const MenuItem& Var2) {return Var1.Proteins > Var2.Proteins;}
    static bool CompareByFats(const MenuItem& Var1, const MenuItem& Var2) {return Var1.Fats > Var2.Fats;}
    static bool CompareByPrice(const MenuItem& Var1, const MenuItem& Var2) {return Var1.Price > Var2.Price;}
    static bool CompareByFactor(const MenuItem& Var1, const MenuItem& Var2) {return Var1.Factor > Var2.Factor;}
};

unsigned int MenuItem::Counter = 0u;

struct DailyRation
{
    std::vector<MenuItem> Meals;
    std::map<std::string, int> Categories;
    std::vector<float> TotalPrice;
    std::vector<float> TotalCalories;
    std::vector<float> TotalCarbohydrates;
    std::vector<float> TotalProteins;
    std::vector<float> TotalFats;

    DailyRation(const std::vector<MenuItem>& MenuItems)
    {
        for (auto& Item : MenuItems)
        {
            Push(Item);
        }
    }

    DailyRation() {}

    DailyRation& Push(const MenuItem& Item)
    {
        ++Categories[Item.Category];
        if (Meals.size() > 0)
        {
            TotalPrice.push_back(TotalPrice.back() + Item.Price);
            TotalCalories.push_back(TotalCalories.back() + Item.TotalCalories);
            TotalCarbohydrates.push_back(TotalCarbohydrates.back() + Item.Carbohydrates);
            TotalProteins.push_back(TotalProteins.back() + Item.Proteins);
            TotalFats.push_back(TotalFats.back() + Item.Fats);
        }
        else
        {
            TotalPrice.push_back(Item.Price);
            TotalCalories.push_back(Item.TotalCalories);
            TotalCarbohydrates.push_back(Item.Carbohydrates);
            TotalProteins.push_back(Item.Proteins);
            TotalFats.push_back(Item.Fats);
        }
        Meals.push_back(Item);
        return *this;
    }

    DailyRation& Pop()
    {
        if (Meals.size() == 0)
        {
            throw std::runtime_error("Array is empty.");
        }

        --Categories[Meals.back().Category];
        TotalCalories.pop_back();
        TotalPrice.pop_back();
        TotalCarbohydrates.pop_back();
        TotalProteins.pop_back();
        TotalFats.pop_back();
        Meals.pop_back();

        return *this;
    }

    void Print()
    {
        for (std::size_t i = 0; i < Meals.size(); ++i)
        {
            std::cout << "    " << i << ": " << Meals[i].Name << " " << Meals[i].AdditionalName << " : " << Meals[i].Id << std::endl;
        }
        std::cout << "    Total calories: " << TotalCalories.back() << std::endl;
        std::cout << "    Total price: " << TotalPrice.back() << std::endl;
        std::cout << "    Factor: " << GetFactor() << std::endl;
    }

    float GetFactor() const
    {
        if (Meals.size() > 0)
        {
            return TotalCalories.back() / TotalPrice.back();
        }
        return 0.f;
    }

    bool operator==(const DailyRation& Other)
    {
        std::map<std::uint32_t, int> MealMap;
        for (auto& Meal : Meals)
        {
            ++MealMap[Meal.Id];
        }
        for(auto& Meal : Other.Meals)
        {
            --MealMap[Meal.Id];
        }
        for (auto& M : MealMap)
        {
            if (M.second != 0)
                return false;
        }
        return true;
    }

    static bool CompareByTotalCalories(const DailyRation& Var1, const DailyRation& Var2) {return Var1.TotalCalories.back() > Var2.TotalCalories.back();}
    static bool CompareByCarbohydrates(const DailyRation& Var1, const DailyRation& Var2) {return Var1.TotalCarbohydrates.back() > Var2.TotalCarbohydrates.back();}
    static bool CompareByProteins(const DailyRation& Var1, const DailyRation& Var2) {return Var1.TotalProteins.back() > Var2.TotalProteins.back();}
    static bool CompareByFats(const DailyRation& Var1, const DailyRation& Var2) {return Var1.TotalFats.back() > Var2.TotalFats.back();}
    static bool CompareByPrice(const DailyRation& Var1, const DailyRation& Var2) {return Var1.TotalPrice.back() > Var2.TotalPrice.back();}
    static bool CompareByFactor(const DailyRation& Var1, const DailyRation& Var2) {return Var1.GetFactor() > Var2.GetFactor();}

    auto Size() {return Meals.size();}

    auto TotalOfCategory(const std::string& Category) {return Categories[Category];}

};

void RecursiveComposition(std::vector<DailyRation>& DailyRations, const std::vector<MenuItem>& Menu, size_t StartingIndex, DailyRation &Ration)
{
    for (std::size_t i = StartingIndex; i < Menu.size(); ++i)
    {
        Ration.Push(Menu[i]);

        // If DailyRation has more calories than we need
        // Or if DailyRation has more dishes than needed
        // Or if it's second dessert
        if ((Ration.TotalCalories.back() > MaximumMealCalories) ||
        (Ration.Size() >= MaxMealsPerDay) ||
        (Ration.TotalOfCategory("drink") + Ration.TotalOfCategory("snack") + Ration.TotalOfCategory("bread") > 1))
        {
            Ration.Pop();
            continue;
        }
        // If Meal has more calories that minimum then we need to check it
        if (Ration.TotalCalories.back() > MinimumMealCalories)
        {
            // If meal's factor fits then we save it
            if (Ration.GetFactor() > MinMealFactor && Ration.Size() >= MinMealPerDay)
            {
                DailyRations.push_back(Ration);
            }
            Ration.Pop();
            continue;
        }

        RecursiveComposition(DailyRations, Menu, i + 1, Ration);

        Ration.Pop();
    }

    return;
}

std::vector<DailyRation> GenerateWeeklyRation(std::vector<DailyRation>& Solutions)
{
    std::vector<DailyRation> WeeklyRation;

    for (int day = 0; day < 5; ++day)
    {
        std::random_device RandomDevice;
        std::mt19937 Generator{RandomDevice()};
        std::uniform_int_distribution<int> Distribution(0,int(Solutions.size() - 1));
        auto RandomDailyMeal = std::bind(Distribution, Generator);

        int Index = RandomDailyMeal();
        auto DailyRation = Solutions[Index];
        WeeklyRation.push_back(DailyRation);

        for (auto Meal : DailyRation.Meals)
        {
            for(int i = 0; i < Solutions.size(); ++i)
            {
                for (auto Single : Solutions[i].Meals)
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
            if (Menu[i].Name.find(DishName) != std::string::npos || Menu[i].AdditionalName.find(DishName) != std::string::npos)
            {
                Menu[i] = Menu.back();
                Menu.pop_back();
                --i;
                continue;
            }
        }
    }

    // Sort so the drinks, bread and snacks are first
    std::size_t l = 0;
    std::size_t r = Menu.size() - 1;
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

    if (false)
    {
        // Sort by some parameter and print data
        std::sort(Menu.begin(), Menu.end(), MenuItem::CompareByCarbohydrates);
        for (auto& Item : Menu)
        {
            std::cout << Item.Carbohydrates << ": " << Item.Name << " " << Item.AdditionalName << std::endl;
        }
        return 0;
    }

    std::cout<< "Menu enlists " << Menu.size() << " positions." << std::endl;

    std::vector<DailyRation> Solutions;
    DailyRation Ration;
    RecursiveComposition(Solutions, Menu, 0, Ration);


    std::cout << "Total of " << Solutions.size() << " daily rations found." << std::endl;
    for (int i = 0; i <= MaxMealsPerDay; ++i)
    {
        int j = 0;
        for (auto& Solution : Solutions)
        {
            if (Solution.Size() == i)
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
        for (int j = 0; j < WeeklyRation[i].Size(); ++j)
        {
            OutputFile << j+1 << ": " << WeeklyRation[i].Meals[j].Name << " " << WeeklyRation[i].Meals[j].AdditionalName << std::endl;
            DailyCalories += WeeklyRation[i].Meals[j].TotalCalories;
            DailyPrice += WeeklyRation[i].Meals[j].Price;
        }
        OutputFile << "Total daily calories:" << DailyCalories << std::endl;
        OutputFile << "Total price: " << DailyPrice << std::endl;
        OutputFile << "Factor: " << DailyCalories / DailyPrice << std::endl << std::endl;
    }
    OutputFile.close();

    return 0;
}