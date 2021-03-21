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

struct FOptions
{
    float MaximumMealCalories = 2000.f;
    float MinimumMealCalories = 1900.f;
    int Days = 5;
    float MaxDailyPrice = 550.f;
    int MinMealsPerDay = 4;
    int MaxMealsPerDay = 5;
    float Factor = MinimumMealCalories / MaxDailyPrice;
    std::vector<std::string> Categories = {"breakfast", "salad", "soup", "main_dish", "drink", "bread", "snack"};
    std::vector<std::string> Exceptions = {"Хлеб", "хлеб", "Булочка", "булочка", "Тартин", "тартин", "Оливье",
                                           "оливье", "Пирожки", "пирожки", "Торт", "торт", "Напиток", "напиток", "Сэндвич",
                                           "сэндвич", "Бургер", "бургер", "Капрезе", "капрезе", "Кус-кус", "кус-кус",
                                           "Булгур", "булгур", "Кекс", "кекс", "Фондан", "фондан"};
    std::vector<std::pair<std::vector<std::string>, int>> Limitations = {{{"drink", "bread", "snack"}, 1},
                                                                         {{"salad"}, 2},
                                                                         {{"breakfast"}, 2},
                                                                         {{"main_dish"}, 2},
                                                                         {{"soup"}, 2}, };
    int MaxMealRepeat = 1;
};

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

    int GetMaxCategory()
    {
        int Max = 0;
        for (auto& Category : Categories)
        {
            if (Category.second > Max)
            {
                Max = Category.second;
            }
        }
        return Max;
    }

    float GetFactor() const
    {
        if (Meals.size() > 0)
        {
            return TotalCalories.back() / TotalPrice.back();
        }
        return 0.f;
    }

    void Print() const
    {
        for (std::size_t i = 0; i < Meals.size(); ++i)
        {
            std::cout << "    " << i << ": " << Meals[i].Name << " " << Meals[i].AdditionalName << " : " << Meals[i].Id << std::endl;
        }
        std::cout << "    Total calories: " << TotalCalories.back() << std::endl;
        std::cout << "    Total price: " << TotalPrice.back() << std::endl;
        std::cout << "    Factor: " << GetFactor() << std::endl;
    }

    std::ostream& operator<<(std::ostream &out)
    {
        for (std::size_t i = 0; i < Meals.size(); ++i)
        {
            out << "    " << i << ": " << Meals[i].Name << " " << Meals[i].AdditionalName << " : " << Meals[i].Id << std::endl;
        }
        out << "    Total calories: " << TotalCalories.back() << std::endl;
        out << "    Total price: " << TotalPrice.back() << std::endl;
        out << "    Factor: " << GetFactor() << std::endl;
        return out;
    }

    bool operator==(const DailyRation& Other) const
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

    auto Size() {return Meals.size();} const

    auto TotalOfCategory(const std::string& Category) const
    {
        if (Categories.find(Category) != Categories.end()) {
            return Categories.at(Category);
        }
        return 0;
    }

};

void RecursiveComposition(std::vector<DailyRation>& DailyRations, const std::vector<MenuItem>& Menu, size_t StartingIndex, DailyRation &Ration, const FOptions& Options)
{
    for (std::size_t i = StartingIndex; i < Menu.size(); ++i)
    {
        Ration.Push(Menu[i]);

        // If DailyRation has more calories than we need
        // Or if DailyRation has more dishes than needed
        if ((Ration.TotalCalories.back() > Options.MaximumMealCalories) ||
        (Ration.Size() >= Options.MaxMealsPerDay))
        {
            Ration.Pop();
            continue;
        }

        bool Found = false;
        for (auto& Limitation : Options.Limitations)
        {
            int Count = 0;
            for (auto& Category : Limitation.first)
            {
                Count+= Ration.TotalOfCategory(Category);
                if (Count > Limitation.second)
                {
                    Found = true;
                    break;
                }
            }
            if (Found)
            {
                break;
            }
        }
        if (Found)
        {
            Ration.Pop();
            continue;
        }

        // If Meal has more calories than minimum that we need to check it
        if (Ration.TotalCalories.back() > Options.MinimumMealCalories)
        {
            // If meal's factor fits then we save it
            if (Ration.GetFactor() > Options.Factor && Ration.Size() >= Options.MinMealsPerDay)
            {
                DailyRations.push_back(Ration);
            }
            Ration.Pop();
            continue;
        }

        RecursiveComposition(DailyRations, Menu, i + 1, Ration, Options);

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

void from_json(const json& Json, FOptions& Options)
{
    Json.at("MaxCalories").get_to(Options.MaximumMealCalories);
    Json.at("MinCalories").get_to(Options.MinimumMealCalories);
    Json.at("Days").get_to(Options.Days);
    Json.at("MinMealsPerDay").get_to(Options.MinMealsPerDay);
    Json.at("MaxMealsPerDay").get_to(Options.MaxMealsPerDay);
    Json.at("MaxDailyPrice").get_to(Options.MaxDailyPrice);
    Json.at("MaxMealRepeat").get_to(Options.MaxMealRepeat);
    Options.Categories.resize(0);
    Json.at("Categories").get_to(Options.Categories);\
    Options.Exceptions.resize(0);
    Json.at("Exceptions").get_to(Options.Exceptions);
    json Limitations = Json.at("Limitations");
    Options.Limitations.resize(0);
    for (auto& Limitation : Limitations)
    {
        std::vector<std::string> CategoryLimitation;
        auto Value = std::stoi(Limitation["Count"].get<std::string>());
        for(auto& Str : Limitation["Categories"])
        {
            CategoryLimitation.push_back(Str.get<std::string>());
        }
        Options.Limitations.push_back({CategoryLimitation, Value});
    }
    Options.Factor = Options.MinimumMealCalories / Options.MaxDailyPrice;
}

int main(int argc, char* argv[])
{
    system("chcp 65001");

    std::vector<MenuItem> Menu;

    // Load menu
    std::ifstream MenuFile("../Menu.json", std::ifstream::in);
    if (!MenuFile.is_open())
        return -1;
    json JsonMenu;
    MenuFile >> JsonMenu;
    for (auto JsonMenuItem : JsonMenu["Menu"])
    {
        MenuItem Item = JsonMenuItem.get<MenuItem>();
        Menu.push_back(Item);
    }

    // Load options
    std::ifstream OptionsFile("../Options.json", std::ifstream::in);
    if (!OptionsFile.is_open())
        return -1;
    json JsonOptions;
    OptionsFile >> JsonOptions;
    FOptions Options = JsonOptions["Options"];

    // Filter Menu
    Menu.erase(std::remove_if(Menu.begin(), Menu.end(), [&Options](const MenuItem& Item)
    {
        // It that lambda returns true then item will be removed
        // If Item is not available
        if (!Item.Available)
        {
            return true;
        }

        // If it and exception
        for (auto& Exception : Options.Exceptions)
        {
            if (Item.Name.find(Exception) != std::string::npos || Item.AdditionalName.find(Exception) != std::string::npos)
                return true;
        }

        // If Item is from category we don't need
        for (auto& Category :Options.Categories)
        {
            if (Category == Item.Category)
            {
                return false;
            }
        }
        return true;
    }), Menu.end());


    // Sort the menu by category, taking into account Limitations. This should speed up recursive generation process.
    std::size_t p = 0;
    for (auto& Limitation : Options.Limitations)
    {
        std::size_t l = p;
        std::size_t r = Menu.size() - 1;
        while (l < r)
        {
            while (l < r)
            {
                bool found = false;
                for(auto& Category : Limitation.first)
                {
                    if (Category == Menu[l].Category)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    break;
                ++l;
            }
            while (l < r)
            {
                bool found = false;
                for(auto& Category : Limitation.first)
                {
                    if (Category == Menu[r].Category)
                    {
                        found = true;
                        break;
                    }
                }
                if (found)
                    break;
                --r;
            }
            auto T = Menu[l];
            Menu[l] = Menu[r];
            Menu[r] = T;
            p = l;
        }
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
    auto Start = std::chrono::steady_clock::now();
    RecursiveComposition(Solutions, Menu, 0, Ration, Options);
    auto End = std::chrono::steady_clock::now();
    std::cout<<"Recursive time: " << std::chrono::duration<float>(End-Start).count() << std::endl;

    std::cout << "Total of " << Solutions.size() << " daily rations found." << std::endl;
    std::map<int, std::size_t> Dispersion;
    for(auto& Solution : Solutions)
    {
        ++Dispersion[Solution.Size()];
    }
    for (auto& Iter : Dispersion)
    {
        std::cout << Iter.first << "-meal solutions: " << Iter.second << std::endl;
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