import requests
from bs4 import BeautifulSoup

desired_calories = 1900
desired_daily_price = 500
max_dessert_calories = 500

class Dish:
    def __lt__(self, other):
        return self.factor < other.factor
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
                break
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
            dish.total_calories = dish.weight * dish.calories_per_100
            dish.factor = dish.total_calories / dish.price / 100.0
            dishes.append(dish)


    dishes.sort()
    for dish in dishes:
        print(dish.name + ' : ' + str(dish.factor))


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    generate_courses()

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
