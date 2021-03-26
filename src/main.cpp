#include "main.h"
#include "json.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <functional>
#include <chrono>
#include <map>
#include <iomanip>

using json = nlohmann::json;

namespace Options
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
    std::vector<std::string> Required = {"Лазанья"};
    std::vector<std::pair<std::vector<std::string>, int>> Limitations = {{{"drink", "bread", "snack"}, 1},
                                                                         {{"salad"}, 2},
                                                                         {{"breakfast"}, 2},
                                                                         {{"main_dish"}, 2},
                                                                         {{"soup"}, 2}, };
    int MaxMealRepeat = 1;

    const float ProteinCalories = 4.1f;
    const float FatsCalories = 9.1f;
    const float CarbohydratesCalories = 4.1f;
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

    float GetTotalPrice() const {return TotalPrice.back();}
    float GetTotalCalories() const {return TotalCalories.back();}
    float GetTotalCarbohydrates() const {return TotalCarbohydrates.back();}
    float GetTotalProteins() const {return TotalProteins.back();}
    float GetTotalFats() const {return TotalFats.back();}

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
            TotalCarbohydrates.push_back(TotalCarbohydrates.back() + Item.Carbohydrates * Item.Weight / 100.f);
            TotalProteins.push_back(TotalProteins.back() + Item.Proteins * Item.Weight / 100.f);
            TotalFats.push_back(TotalFats.back() + Item.Fats * Item.Weight / 100.f);
        }
        else
        {
            TotalPrice.push_back(Item.Price);
            TotalCalories.push_back(Item.TotalCalories);
            TotalCarbohydrates.push_back(Item.Carbohydrates * Item.Weight / 100.f);
            TotalProteins.push_back(Item.Proteins * Item.Weight / 100.f);
            TotalFats.push_back(Item.Fats * Item.Weight / 100.f);
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

    int GetMaxCategory() const
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

    auto TotalOfCategory(const std::string& Category) const
    {
        if (Categories.find(Category) != Categories.end()) {
            return Categories.at(Category);
        }
        return 0;
    }

    static bool CompareByTotalCalories(const DailyRation& Var1, const DailyRation& Var2) {return Var1.TotalCalories.back() > Var2.TotalCalories.back();}
    static bool CompareByCarbohydrates(const DailyRation& Var1, const DailyRation& Var2) {return Var1.TotalCarbohydrates.back() > Var2.TotalCarbohydrates.back();}
    static bool CompareByProteins(const DailyRation& Var1, const DailyRation& Var2) {return Var1.TotalProteins.back() > Var2.TotalProteins.back();}
    static bool CompareByFats(const DailyRation& Var1, const DailyRation& Var2) {return Var1.TotalFats.back() > Var2.TotalFats.back();}
    static bool CompareByPrice(const DailyRation& Var1, const DailyRation& Var2) {return Var1.TotalPrice.back() > Var2.TotalPrice.back();}
    static bool CompareByFactor(const DailyRation& Var1, const DailyRation& Var2) {return Var1.GetFactor() > Var2.GetFactor();}

    inline auto Size() const {return Meals.size();}
    MenuItem& operator[](std::size_t Index)
    {
        return Meals[Index];
    }

};

struct RationsStorage
{
    std::vector<DailyRation> Storage;

    RationsStorage& Push(const DailyRation& Ration)
    {
        Storage.push_back(Ration);
        return *this;
    }

    RationsStorage& Pop()
    {
        Storage.pop_back();
        return *this;
    }

    auto Size() const {return Storage.size();}

    auto begin() const {return Storage.begin();}
    auto end() const {return Storage.end();}
    auto back() const {return Storage.back();}

    DailyRation& operator[](std::size_t Index)
    {
        return Storage[Index];
    }
};

std::ostream& operator<<(std::ostream &out, DailyRation& Ration)
{
    for (std::size_t i = 0; i < Ration.Size(); ++i)
    {
        out << " " << i+1 << ": " << Ration[i].Name << " " << Ration[i].AdditionalName << std::endl;
    }
    out << " P/F/C: " << std::setprecision(3) << Ration.GetTotalProteins() << "/" << Ration.GetTotalFats() << "/" << Ration.GetTotalCarbohydrates() << " ";
    out << " kcal/$=Factor: " << std::setprecision(4) << Ration.TotalCalories.back() << "/" << Ration.TotalPrice.back() << "=" << Ration.GetFactor() << std::endl;
    return out;
}

void RecursiveComposition(RationsStorage& Storage, const std::vector<MenuItem>& Menu, size_t StartingIndex, DailyRation &Ration)
{
    for (std::size_t i = StartingIndex; i < Menu.size(); ++i)
    {
        Ration.Push(Menu[i]);

        // If DailyRation has more calories than we need
        // Or if DailyRation has more dishes than needed
        if ((Ration.TotalCalories.back() > Options::MaximumMealCalories) ||
        (Ration.Size() >= Options::MaxMealsPerDay))
        {
            Ration.Pop();
            continue;
        }

        bool Found = false;
        for (auto& Limitation : Options::Limitations)
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
        if (Ration.TotalCalories.back() > Options::MinimumMealCalories)
        {
            // If meal's factor fits then we save it
            if (Ration.GetTotalPrice() <= Options::MaxDailyPrice && Ration.Size() >= Options::MinMealsPerDay)
            {
                Storage.Push(Ration);
            }
            Ration.Pop();
            continue;
        }

        RecursiveComposition(Storage, Menu, i + 1, Ration);

        Ration.Pop();
    }

    return;
}

std::vector<DailyRation> GenerateWeeklyRation(RationsStorage& Storage)
{
    std::vector<DailyRation> WeeklyRation;

    for (int day = 0; day < 5; ++day)
    {
        std::random_device RandomDevice;
        std::mt19937 Generator{RandomDevice()};
        std::uniform_int_distribution<int> Distribution(0,int(Storage.Size() - 1));
        auto RandomDailyMeal = std::bind(Distribution, Generator);

        int Index = RandomDailyMeal();
        auto DailyRation = Storage[Index];
        WeeklyRation.push_back(DailyRation);

        for (auto Meal : DailyRation.Meals)
        {
            for(int i = 0; i < Storage.Size(); ++i)
            {
                for (auto Single : Storage[i].Meals)
                {
                    if (Single == Meal)
                    {
                        if (Storage.Size() == 0)
                        {
                            // TODO
                            throw std::runtime_error("Out of solutions!");
                        }
                        Storage[i] = Storage.back();
                        Storage.Pop();
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

void LoadOptions(const json& Json)
{
    Json.at("MaxCalories").get_to(Options::MaximumMealCalories);
    Json.at("MinCalories").get_to(Options::MinimumMealCalories);
    Json.at("Days").get_to(Options::Days);
    Json.at("MinMealsPerDay").get_to(Options::MinMealsPerDay);
    Json.at("MaxMealsPerDay").get_to(Options::MaxMealsPerDay);
    Json.at("MaxDailyPrice").get_to(Options::MaxDailyPrice);
    Json.at("MaxMealRepeat").get_to(Options::MaxMealRepeat);
    Options::Categories.resize(0);
    Json.at("Categories").get_to(Options::Categories);
    Options::Exceptions.resize(0);
    Json.at("Exceptions").get_to(Options::Exceptions);
    Options::Required.resize(0);
    Json.at("Required").get_to(Options::Required);
    json Limitations = Json.at("Limitations");
    Options::Limitations.resize(0);
    for (auto& Limitation : Limitations)
    {
        std::vector<std::string> CategoryLimitation;
        auto Value = std::stoi(Limitation["Count"].get<std::string>());
        for(auto& Str : Limitation["Categories"])
        {
            CategoryLimitation.push_back(Str.get<std::string>());
        }
        Options::Limitations.push_back({CategoryLimitation, Value});
    }
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
    LoadOptions(JsonOptions["Options"]);

    // Filter Menu
    Menu.erase(std::remove_if(Menu.begin(), Menu.end(), [](const MenuItem& Item)
    {
        // It that lambda returns true then item will be removed
        // If Item is not available
        if (!Item.Available)
        {
            return true;
        }

        // If it and exception
        for (auto& Exception : Options::Exceptions)
        {
            if (Item.Name.find(Exception) != std::string::npos || Item.AdditionalName.find(Exception) != std::string::npos)
                return true;
        }

        // If Item is from category we don't need
        for (auto& Category :Options::Categories)
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
    for (auto& Limitation : Options::Limitations)
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

    std::cout<< "Menu enlists " << Menu.size() << " positions." << std::endl;

    RationsStorage Solutions;
    DailyRation Ration;
    auto Start = std::chrono::steady_clock::now();
    RecursiveComposition(Solutions, Menu, 0, Ration);
    auto End = std::chrono::steady_clock::now();
    std::cout<<"Recursive time: " << std::chrono::duration<float>(End-Start).count() << std::endl;

    std::cout << "Total of " << Solutions.Size() << " daily rations found." << std::endl;
    std::map<std::size_t, std::size_t> Dispersion;
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
        OutputFile << "Day " << i+1 << std::endl;
        OutputFile << WeeklyRation[i] << std::endl;
    }
    OutputFile.close();

    return 0;
}