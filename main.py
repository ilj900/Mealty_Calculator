import operator
import requests
import subprocess
from bs4 import BeautifulSoup

desired_calories = 1900
maximum_calories = 2200
desired_daily_price = 500
max_dessert_calories = 600
max_desserts_in_meal = 1
max_bread_calories = 600


class Dish:
    name: str
    category: str
    weight: float
    calories_per_100: float
    total_calories: float
    carbohydrates: float
    proteins: float
    fats: float
    price: float
    factor: float


def generate_courses():
    # Load paige and create soup
    mealy_url = 'https://www.mealty.ru/catalog/'
    html_text = requests.get(mealy_url).text
    soup = BeautifulSoup(html_text, 'html.parser')

    # Some data used to cut out courses we are not interested in
    categories_of_interest = list()
    categories_of_interest.append('breakfast')
    categories_of_interest.append('salad')
    categories_of_interest.append('soup')
    categories_of_interest.append('main_dish')
    categories_of_interest.append('drink')
    categories_of_interest.append('bread')
    exceptions = ['Хлеб', 'Тартин', 'Булочка']

    dishes = []

    for category in categories_of_interest:
        category_entrance = soup.find('div', {'class': ['landingItem'], 'id': category})
        all_category_meals = category_entrance.find_all('div', class_='meal-card')
        all_category_prices = category_entrance.find_all('div', class_='meal-card__controls')
        for meal_data, price_data in zip(all_category_meals, all_category_prices):
            if price_data.find('div', class_="meal-card__buttons out-of-stock-hide hidden"):
                continue
            main_name = meal_data.find('div', class_='meal-card__name').text
            found = False
            for exception in exceptions:
                if exception in main_name or exception.lower() in main_name:
                    found = True
                    break
            if found:
                continue
            second_name = meal_data.find('div', class_='meal-card__name-note').text

            dish = Dish()
            dish.name = meal_data.find('div', class_='meal-card__name').text
            if len(second_name) > 0:
                dish.name += ' ' + second_name
            dish.weight = float(meal_data.find('div', class_='meal-card__weight').text)
            dish.category = category
            dish.carbohydrates = float(meal_data.find('div', class_='meal-card__carbohydrates').text.replace(',', '.'))
            dish.proteins = float(meal_data.find('div', class_='meal-card__proteins').text.replace(',', '.'))
            dish.fats = float(meal_data.find('div', class_='meal-card__fats').text.replace(',', '.'))
            dish.calories_per_100 = float(meal_data.find('div', class_='meal-card__calories').text)
            dish.price = float(price_data.find('span', class_='basket__footer-total-count green').text)
            dish.total_calories = dish.weight * dish.calories_per_100 / 100.0
            dish.factor = dish.total_calories / dish.price
            dishes.append(dish)

    dishes.sort(key=operator.attrgetter('total_calories'))
    print(str(len(dishes)) + ' dishes in total.')
    for dish in dishes:
        if dish.category == 'bread':
            print(dish.name + ' : ' + str(dish.factor))

    # Generate parameters string and run subprocess
    names_string = ['--Names']
    categories_string = ['--Categories']
    weights_string = ['--Weights']
    calories_per_100_string = ['--CaloriesPer100']
    total_calories_string = ['--TotalCalories']
    carbohydrates_string = ['--Carbohydrates']
    proteins_string = ['--Proteins']
    fats_string = ['--Fats']
    prices_string = ['--Prices']
    factors_string = ['--Factors']
    for dish in dishes:
        names_string += dish.name
        categories_string += dish.category
        weights_string += "{:.2f}".format(dish.weight)
        calories_per_100_string += "{:.2f}".format(dish.calories_per_100)
        total_calories_string += "{:.2f}".format(dish.total_calories)
        carbohydrates_string += "{:.2f}".format(dish.carbohydrates)
        proteins_string += "{:.2f}".format(dish.proteins)
        fats_string += "{:.2f}".format(dish.fats)
        prices_string += "{:.2f}".format(dish.price)
        factors_string += "{:.2f}".format(dish.factor)

    args = ["MealMe.exe"]
    args.extend(names_string)
    args.extend(categories_string)
    args.extend(weights_string)
    args.extend(calories_per_100_string)
    args.extend(total_calories_string)
    args.extend(categories_string)
    args.extend(proteins_string)
    args.extend(fats_string)
    args.extend(prices_string)
    args.extend(factors_string)

    process = subprocess.run(args, shell=False, timeout=300)



# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    generate_courses()

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
