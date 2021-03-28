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

using json = nlohmann::ordered_json;

namespace Options
{
    float MaximumMealCalories = 2000.f;
    float MinimumMealCalories = 1900.f;
    int Days = 5;
    float MaxDailyPrice = 550.f;
    int MinMealsPerDay = 4;
    int MaxMealsPerDay = 5;
    std::vector<std::string> Categories;
    std::vector<std::string> Exceptions;
    std::vector<std::string> Required;
    std::vector<std::pair<std::vector<std::string>, int>> Limitations;
    int MaxMealRepeat = 1;

    const float ProteinCalories = 4.1f;
    const float FatsCalories = 9.1f;
    const float CarbohydratesCalories = 4.1f;

    void LoadOptions(const std::string& Path)
    {
        // Load options
        std::ifstream OptionsFile(Path, std::ifstream::in);
        if (!OptionsFile.is_open())
            throw std::runtime_error("Failed to load options from file");
        json JsonFile;
        OptionsFile >> JsonFile;
        auto JsonOptions = JsonFile["Options"];

        JsonOptions.at("MaxCalories").get_to(Options::MaximumMealCalories);
        JsonOptions.at("MinCalories").get_to(Options::MinimumMealCalories);
        JsonOptions.at("Days").get_to(Options::Days);
        JsonOptions.at("MinMealsPerDay").get_to(Options::MinMealsPerDay);
        JsonOptions.at("MaxMealsPerDay").get_to(Options::MaxMealsPerDay);
        JsonOptions.at("MaxDailyPrice").get_to(Options::MaxDailyPrice);
        JsonOptions.at("MaxMealRepeat").get_to(Options::MaxMealRepeat);
        Options::Categories.resize(0);
        JsonOptions.at("Categories").get_to(Options::Categories);
        Options::Exceptions.resize(0);
        JsonOptions.at("Exceptions").get_to(Options::Exceptions);
        Options::Required.resize(0);
        JsonOptions.at("Required").get_to(Options::Required);
        json Limitations = JsonOptions.at("Limitations");
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
};

struct FMenuItem
{
public:
    FMenuItem(): Id(Counter++) {};

    static unsigned int Counter;

    bool operator==(const FMenuItem& Other) {return Other.Id == Id;}
    static bool CompareByWeight(const FMenuItem& Var1, const FMenuItem& Var2) {return Var1.Weight > Var2.Weight;}
    static bool CompareByCaloriesPer100(const FMenuItem& Var1, const FMenuItem& Var2) {return Var1.CaloriesPer100 > Var2.CaloriesPer100;}
    static bool CompareByTotalCalories(const FMenuItem& Var1, const FMenuItem& Var2) {return Var1.TotalCalories > Var2.TotalCalories;}
    static bool CompareByCarbohydrates(const FMenuItem& Var1, const FMenuItem& Var2) {return Var1.Carbohydrates > Var2.Carbohydrates;}
    static bool CompareByProteins(const FMenuItem& Var1, const FMenuItem& Var2) {return Var1.Proteins > Var2.Proteins;}
    static bool CompareByFats(const FMenuItem& Var1, const FMenuItem& Var2) {return Var1.Fats > Var2.Fats;}
    static bool CompareByPrice(const FMenuItem& Var1, const FMenuItem& Var2) {return Var1.Price > Var2.Price;}
    static bool CompareByFactor(const FMenuItem& Var1, const FMenuItem& Var2) {return Var1.Factor > Var2.Factor;}

    inline bool Find(const std::string& Word) const
    {
        if (Name.find(Word) != std::string::npos || AdditionalName.find(Word) != std::string::npos)
        {
            return true;
        }
        return false;
    }

    inline bool FindAny(const std::vector<std::string>& Words) const
    {
        for (auto& Word : Words)
        {
            if (Find(Word))
            {
                return true;
            }
        }
        return false;
    }

    inline bool FindAll(const std::vector<std::string>& Words) const
    {
        for (auto& Word : Words)
        {
            if (!Find(Word))
            {
                return false;
            }
        }
        return true;
    }

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
};

unsigned int FMenuItem::Counter = 0u;

void from_json(const json& Json, FMenuItem& MenuItem)
{
    Json.at("name").get_to(MenuItem.Name);
    Json.at("additional_name").get_to(MenuItem.AdditionalName);
    Json.at("category").get_to(MenuItem.Category);
    Json.at("weight").get_to(MenuItem.Weight);
    Json.at("calories_per_100").get_to(MenuItem.CaloriesPer100);
    Json.at("total_calories").get_to(MenuItem.TotalCalories);
    Json.at("carbohydrates").get_to(MenuItem.Carbohydrates);
    Json.at("proteins").get_to(MenuItem.Proteins);
    Json.at("fats").get_to(MenuItem.Fats);
    Json.at("price").get_to(MenuItem.Price);
    Json.at("factor").get_to(MenuItem.Factor);
    Json.at("available").get_to(MenuItem.Available);
}

void to_json(json& Json, const FMenuItem& MenuItem)
{
    Json = json{{"name", MenuItem.Name},
                {"additional_name", MenuItem.AdditionalName},
                {"category", MenuItem.Category},
                {"weight", MenuItem.Weight},
                {"calories_per_100", MenuItem.CaloriesPer100},
                {"total_calories", MenuItem.TotalCalories},
                {"carbohydrates", MenuItem.Carbohydrates},
                {"proteins", MenuItem.Proteins},
                {"fats", MenuItem.Fats},
                {"price", MenuItem.Price},
                {"factor", MenuItem.Factor},
                {"available", MenuItem.Available}
    };
}

struct FMenu
{
    std::vector<FMenuItem> Data;

    FMenu& Push(const FMenuItem& MenuItem)
    {
        Data.push_back(MenuItem);
        return *this;
    }

    FMenu& Pop()
    {
        Data.pop_back();
        return *this;
    }

    auto Size() const {return Data.size();}
    inline FMenuItem& operator[](std::size_t Index)
    {
        return Data[Index];
    }

    int SaveMenuToFile(const std::string& Path)
    {
        json JsonMenu = json::array();
        for (auto& Item : Data)
        {
            JsonMenu.push_back(Item);
        }
        json JsonFile;
        JsonFile["Menu"] = JsonMenu;

        // Write down the data
        std::ofstream OutputFile;
        OutputFile.open(Path);
        if (!OutputFile.is_open())
        {
            return -1;
        }

        OutputFile << std::setprecision(2) << JsonFile.dump(2)<< std::endl;

        OutputFile.close();
        return 0;
    }

    static FMenu LoadMenuFromFile(const std::string& Path)
    {
        std::ifstream MenuFile(Path, std::ifstream::in);
        if (!MenuFile.is_open())
        {
            throw std::runtime_error("Failed to load menu from file");
        }
        json JsonMenu;
        MenuFile >> JsonMenu;
        FMenu Menu;
        for (auto JsonMenuItem : JsonMenu["Menu"])
        {
            FMenuItem MenuItem = JsonMenuItem.get<FMenuItem>();
            Menu.Push(MenuItem);
        }
        MenuFile.close();
        return Menu;
    }
};

struct FDailyRation
{
    FDailyRation() {}

    FDailyRation(const std::vector<FMenuItem>& MenuItems)
    {
        for (auto& MenuItem : MenuItems)
        {
            Push(MenuItem);
        }
    }

    float GetTotalPrice() const {return TotalPrice.back();}
    float GetTotalCalories() const {return TotalCalories.back();}
    float GetTotalCarbohydrates() const {return TotalCarbohydrates.back();}
    float GetTotalProteins() const {return TotalProteins.back();}
    float GetTotalFats() const {return TotalFats.back();}

    FDailyRation& Push(const FMenuItem& MenuItem)
    {
        ++Categories[MenuItem.Category];
        if (Meals.size() > 0)
        {
            TotalPrice.push_back(TotalPrice.back() + MenuItem.Price);
            TotalCalories.push_back(TotalCalories.back() + MenuItem.TotalCalories);
            TotalCarbohydrates.push_back(TotalCarbohydrates.back() + MenuItem.Carbohydrates * MenuItem.Weight / 100.f);
            TotalProteins.push_back(TotalProteins.back() + MenuItem.Proteins * MenuItem.Weight / 100.f);
            TotalFats.push_back(TotalFats.back() + MenuItem.Fats * MenuItem.Weight / 100.f);
        }
        else
        {
            TotalPrice.push_back(MenuItem.Price);
            TotalCalories.push_back(MenuItem.TotalCalories);
            TotalCarbohydrates.push_back(MenuItem.Carbohydrates * MenuItem.Weight / 100.f);
            TotalProteins.push_back(MenuItem.Proteins * MenuItem.Weight / 100.f);
            TotalFats.push_back(MenuItem.Fats * MenuItem.Weight / 100.f);
        }
        Meals.push_back(MenuItem);
        return *this;
    }

    FDailyRation& Pop()
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

    // Returns how many items of the category are currently in ration
    auto TotalOfCategory(const std::string& Category) const
    {
        if (Categories.find(Category) != Categories.end()) {
            return Categories.at(Category);
        }
        return 0;
    }

    float GetFactor() const
    {
        if (Meals.size() > 0)
        {
            return TotalCalories.back() / TotalPrice.back();
        }
        return 0.f;
    }

    // Two rations are equal if they have the same meals.
    // Order doesn't matter in that case
    bool operator==(const FDailyRation& Other) const
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

    static bool CompareByTotalCalories(const FDailyRation& Var1, const FDailyRation& Var2) {return Var1.TotalCalories.back() > Var2.TotalCalories.back();}
    static bool CompareByCarbohydrates(const FDailyRation& Var1, const FDailyRation& Var2) {return Var1.TotalCarbohydrates.back() > Var2.TotalCarbohydrates.back();}
    static bool CompareByProteins(const FDailyRation& Var1, const FDailyRation& Var2) {return Var1.TotalProteins.back() > Var2.TotalProteins.back();}
    static bool CompareByFats(const FDailyRation& Var1, const FDailyRation& Var2) {return Var1.TotalFats.back() > Var2.TotalFats.back();}
    static bool CompareByPrice(const FDailyRation& Var1, const FDailyRation& Var2) {return Var1.TotalPrice.back() > Var2.TotalPrice.back();}
    static bool CompareByFactor(const FDailyRation& Var1, const FDailyRation& Var2) {return Var1.GetFactor() > Var2.GetFactor();}

    inline auto Size() const {return Meals.size();}
    inline FMenuItem& operator[](std::size_t Index)
    {
        return Meals[Index];
    }

    inline bool Find(const std::string& Word) const
    {
        for(auto& MenuItem : Meals)
        {
            if (MenuItem.Find(Word))
            {
                return true;
            }
        }
        return false;
    }

    inline bool FindAny(const std::vector<std::string>& Words) const
    {
        for(auto& MenuItem : Meals)
        {
            if (MenuItem.FindAny(Words))
            {
                return true;
            }
        }
        return false;
    }

    inline bool FindAll(const std::vector<std::string>& Words) const
    {
        for (auto& Word : Words)
        {
            bool found = false;
            for (auto& MenuItem : Meals)
            {
                if (MenuItem.Find(Word))
                {
                    found = true;
                    break;
                }
            }
            if (found)
            {
                continue;
            }
            return false;
        }
        return true;
    }

    std::vector<FMenuItem> Meals;
    std::map<std::string, int> Categories;
    // We store prices in vector, because too many additions and subtractions will add error to the value
    std::vector<float> TotalPrice;
    std::vector<float> TotalCalories;
    std::vector<float> TotalCarbohydrates;
    std::vector<float> TotalProteins;
    std::vector<float> TotalFats;
};

void to_json(json& Json, const FDailyRation& DailyRation)
{

    Json["Meals"] = json::array();
    Json["Categories"] = json::array();
    Json["TotalPrice"] = json::array();
    Json["TotalCalories"] = json::array();
    Json["TotalCarbohydrates"] = json::array();
    Json["TotalProteins"] = json::array();
    Json["TotalFats"] = json::array();
    for (std::size_t Day = 0; Day < DailyRation.Size(); ++Day)
    {
        Json["Meals"].push_back(DailyRation.Meals[Day]);
        Json["TotalPrice"].push_back(DailyRation.TotalPrice[Day]);
        Json["TotalCalories"].push_back(DailyRation.TotalCalories[Day]);
        Json["TotalCarbohydrates"].push_back(DailyRation.TotalCarbohydrates[Day]);
        Json["TotalProteins"].push_back(DailyRation.TotalProteins[Day]);
        Json["TotalFats"].push_back(DailyRation.TotalFats[Day]);
    }
    Json["Categories"] = DailyRation.Categories;
}

void from_json(const json& Json, FDailyRation& DailyRation)
{
    json J = Json["Meals"];
    DailyRation.Meals.resize(J.size());
    for(std::size_t i = 0; i < J.size(); ++i)
    {
        J.at(i).get_to(DailyRation.Meals[i]);
    }
    Json.at("Meals").get_to(DailyRation.Meals);
    Json.at("TotalPrice").get_to(DailyRation.TotalPrice);
    Json.at("TotalCalories").get_to(DailyRation.TotalCalories);
    Json.at("TotalCarbohydrates").get_to(DailyRation.TotalCarbohydrates);
    Json.at("TotalProteins").get_to(DailyRation.TotalProteins);
    Json.at("TotalFats").get_to(DailyRation.TotalFats);
    DailyRation.Categories = Json.at("Categories").get<std::map<std::string, int>>();
}

struct RationsStorage
{
    RationsStorage& Push(const FDailyRation& DailyRation)
    {
        Storage.push_back(DailyRation);
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

    FDailyRation& operator[](std::size_t Index)
    {
        return Storage[Index];
    }

    int ToFile(const std::string& Path)
    {
        json J = json::array();
        for (auto& DailyRation : Storage)
        {
            J.push_back(DailyRation);
        }
        json JsonFile;
        JsonFile["Storage"] = J;

        // Write down the data
        std::ofstream OutputFile;
        OutputFile.open(Path);
        if (!OutputFile.is_open())
        {
            return -1;
        }

        OutputFile << std::setprecision(2) << JsonFile.dump(2)<< std::endl;

        OutputFile.close();
        return 0;
    }

    static RationsStorage LoadFromFile(const std::string& Path)
    {
        std::ifstream StorageFile(Path, std::ifstream::in);
        if (!StorageFile.is_open())
        {
            throw std::runtime_error("Failed to load storage from file");
        }
        json JsonStorage;
        StorageFile >> JsonStorage;
        RationsStorage Storage;
        for (auto JsonDailyRation : JsonStorage["Storage"])
        {
            FDailyRation MenuItem = JsonDailyRation.get<FDailyRation>();
            Storage.Push(MenuItem);
        }
        StorageFile.close();
        return Storage;
    }

    inline bool Find(const std::string& Word) const
    {
        for (auto& Daily : Storage)
        {
            if (Daily.Find(Word))
            {
                return true;
            }
        }
        return false;
    }

    inline bool FindAny(const std::vector<std::string>& Words) const
    {
        for(auto& Ration : Storage)
        {
            if (Ration.FindAny(Words))
            {
                return true;
            }
        }
        return false;
    }

    inline bool FindAll(const std::vector<std::string>& Words) const
    {
        for(auto& Word : Words)
        {
            bool Found = false;
            for (auto& Ration : Storage)
            {
                if (Ration.Find(Word))
                {
                    Found = true;
                    break;
                }
            }
            if (!Found)
            {
                return false;
            }
        }
        return true;
    }

    std::vector<FDailyRation> Storage;
};

std::ostream& operator<<(std::ostream &out, FDailyRation& DailyRation)
{
    for (std::size_t i = 0; i < DailyRation.Size(); ++i)
    {
        out << " " << i+1 << ": " << DailyRation[i].Name << " " << DailyRation[i].AdditionalName << std::endl;
    }
    out << " P/F/C: " << std::setprecision(3) << DailyRation.GetTotalProteins() << "/" << DailyRation.GetTotalFats() << "/" << DailyRation.GetTotalCarbohydrates() << " ";
    out << " kcal/$=Factor: " << std::setprecision(4) << DailyRation.TotalCalories.back() << "/" << DailyRation.TotalPrice.back() << "=" << DailyRation.GetFactor() << std::endl;
    return out;
}

void RecursiveComposition(RationsStorage& Storage, FMenu& Menu, size_t StartingIndex, FDailyRation &Ration)
{
    for (std::size_t i = StartingIndex; i < Menu.Size(); ++i)
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

RationsStorage GenerateWeeklyRation(RationsStorage& Storage)
{
    RationsStorage WeeklyRation;

    for (int day = 0; day < 5; ++day)
    {
        std::random_device RandomDevice;
        std::mt19937 Generator{RandomDevice()};
        std::uniform_int_distribution<int> Distribution(0,int(Storage.Size() - 1));
        auto RandomDailyMeal = std::bind(Distribution, Generator);

        int Index = RandomDailyMeal();
        auto DailyRation = Storage[Index];
        WeeklyRation.Push(DailyRation);

        for (auto Meal : DailyRation.Meals)
        {
            for(int i = 0; i < Storage.Size(); ++i)
            {
                for (auto Single : Storage[i].Meals)
                {
                    if (Single == Meal)
                    {
                        Storage[i] = Storage.back();
                        Storage.Pop();
                        if (Storage.Size() == 0)
                        {
                            // TODO
                            throw std::runtime_error("Out of solutions!");
                        }
                        --i;
                        break;
                    }
                }
            }
        }
    }

    return WeeklyRation;
}

int main(int argc, char* argv[])
{
    system("chcp 65001");

    FMenu Menu = FMenu::LoadMenuFromFile("../Menu.json");

    Options::LoadOptions("../Options.json");

    // Filter Menu
    Menu.Data.erase(std::remove_if(Menu.Data.begin(), Menu.Data.end(), [](const FMenuItem& MenuItem)
    {
        // It that lambda returns true then item will be removed
        // If Item is not available
        if (!MenuItem.Available)
        {
            return true;
        }

        // If it and exception
        for (auto& Exception : Options::Exceptions)
        {
            if (MenuItem.Name.find(Exception) != std::string::npos || MenuItem.AdditionalName.find(Exception) != std::string::npos)
                return true;
        }

        // If Item is from category we don't need
        for (auto& Category :Options::Categories)
        {
            if (Category == MenuItem.Category)
            {
                return false;
            }
        }
        return true;
    }), Menu.Data.end());


    // Sort the menu by category, taking into account Limitations. This should speed up recursive generation process.
    std::size_t p = 0;
    for (auto& Limitation : Options::Limitations)
    {
        std::size_t l = p;
        std::size_t r = Menu.Size() - 1;
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

    std::cout<< "Menu enlists " << Menu.Size() << " positions." << std::endl;

    RationsStorage Solutions;
    FDailyRation DailyRation;
    auto Start = std::chrono::steady_clock::now();
    RecursiveComposition(Solutions, Menu, 0, DailyRation);
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


    RationsStorage WeeklyRation;
    while(true)
    {
        try
        {
            auto Copy = Solutions;
            WeeklyRation = GenerateWeeklyRation(Copy);
        }
        catch (std::runtime_error)
        {
            continue;
        }
        break;
    }

    std::ofstream OutputFile;
    OutputFile.open("Ration.txt");
    if (!OutputFile.is_open())
    {
        return -1;
    }
    for (int i = 0; i < WeeklyRation.Size(); ++i)
    {
        OutputFile << "Day " << i+1 << std::endl;
        OutputFile << WeeklyRation[i] << std::endl;
    }
    OutputFile.close();

    return 0;
}