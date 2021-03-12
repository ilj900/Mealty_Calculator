import json
import requests
from bs4 import BeautifulSoup


class Dish:
    name: str
    additional_name: str
    category: str
    weight: float
    calories_per_100: float
    total_calories: float
    carbohydrates: float
    proteins: float
    fats: float
    price: float
    factor: float
    available: bool


def extract_data():
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
    categories_of_interest.append('snack')
    categories_of_interest.append('almost_ready')
    categories_of_interest.append('food')

    dishes = []

    for category in categories_of_interest:
        category_entrance = soup.find('div', {'class': ['landingItem'], 'id': category})
        all_category_meals = category_entrance.find_all('div', class_='meal-card')
        all_category_prices = category_entrance.find_all('div', class_='meal-card__controls')
        for meal_data, price_data in zip(all_category_meals, all_category_prices):
            dish = Dish()
            dish.name = meal_data.find('div', class_='meal-card__name').text
            additional_name = meal_data.find('div', class_='meal-card__name-note').text
            if len(additional_name) > 1:
                dish.additional_name = additional_name
            else:
                dish.additional_name = ''
            if price_data.find('div', class_="meal-card__buttons out-of-stock-hide hidden"):
                dish.available = False
            else:
                dish.available = True
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

    # Generate parameters string and save them in file
    dishes_list = []
    for dish in dishes:
        dish_dict = {'name': dish.name, 'additional_name': dish.additional_name, 'category': dish.category,
                     'weight': dish.weight, 'calories_per_100': dish.calories_per_100, 'total_calories': dish.total_calories,
                     'carbohydrates': dish.carbohydrates, 'proteins': dish.proteins, 'fats': dish.fats,
                     'price': dish.price, 'factor': dish.factor, 'available': dish.available}
        dishes_list.append(dish_dict)
    menu = {'Menu': dishes_list}

    try:
        with open('Menu.json', 'w', encoding="utf-8") as file:
            json.dump(menu, file, indent=2, ensure_ascii=False)
    except IOError:
        print('Failed to write file')


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    extract_data()

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
